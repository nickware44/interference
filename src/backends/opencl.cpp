/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     06.04.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/backends/opencl.h"
#include "../../include/inn/neuron.h"
#include "../../include/inn/system.h"

#define KERNEL(name, ...) std::string name = #__VA_ARGS__

inn::ComputeBackendOpenCL::ComputeBackendOpenCL() {
#ifdef INN_OPENCL_SUPPORT
    std::vector<cl::Platform> all_platforms;
    std::vector<cl::Device> all_devices;

    cl::Platform::get(&all_platforms);

    if (all_platforms.empty()) {
        std::cerr << "No platforms found. Check OpenCL installation!" << std::endl;
        return;
    }

    cl::Platform default_platform = all_platforms[0];

    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.empty()) {
        std::cerr << "No devices found. Check OpenCL installation!" << std::endl;;
        return;
    }

    cl::Device default_device = all_devices[0];

    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
    std::cout << "Using device  : " << default_device.getInfo<CL_DEVICE_NAME>() << " (CU: " << default_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << ")" << std::endl;
    std::cout << "Driver version: " << default_device.getInfo<CL_DRIVER_VERSION>() << std::endl;

    KERNEL(kernel_code,
           __kernel void inn_kernel(__global float4 *output,  __global float16 *input) {
                   int id = get_global_id(0);
                   // input.s0 - receptor x
                   // input.s1 - receptor y
                   // input.s2 - synapse x
                   // input.s3 - synapse y

                   // input.s4 - synapse gamma value
                   // input.s5 - neuron input value

                   // input.s6 - lambda
                   // input.s7 - k1
                   // input.s8 - k2

                   // check if we need to compute this pair by lambda value
                   if (!input[id].s6) output[id] = (float4)(0, 0, 0, 0);

                   // vector length
                   float d = 0;
                   d += (input[id].s0-input[id].s2)*(input[id].s0-input[id].s2);
                   d += (input[id].s1-input[id].s3)*(input[id].s1-input[id].s3);
                   d = sqrt(d);

                   float ngamma = input[id].s4 + (input[id].s7*input[id].s5-input[id].s4/input[id].s8);

                   float e = input[id].s6 * exp(-input[id].s6*d);
                   float fi = input[id].s4 * e;
                   float dfi = input[id].s5 * e;
                   float nx = 0, ny = 0;
                   if (dfi > 0) {
                       float nposd = sqrt(dfi) / d;
                       nx = fabs(input[id].s0-input[id].s2) * nposd;
                       ny = fabs(input[id].s1-input[id].s3) * nposd;
                   }

                   output[id] = (float4)(nx, ny, fi, 0);
           }
    );

    Context = cl::Context(CL_DEVICE_TYPE_ALL);

    cl::Program program(Context, {kernel_code.c_str(),kernel_code.length()+1});
    int status;
    if ((status = program.build({default_device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        return;
    }

    Kernel = cl::Kernel(program,"inn_kernel");
    Queue = cl::CommandQueue(Context,default_device);

    Worker = new inn::WorkerInfo;
    Worker -> thread = std::thread(tWorker, Worker, Context, Kernel, Queue);
#else
    std::cerr << "The OpenCL compute backend is not supported by the current build. Rebuild interfernce library with the INN_OPENCL_SUPPORT flag." << std::endl;
#endif
}

void inn::ComputeBackendOpenCL::doRegisterHost(const std::vector<void*> &objects) {
    uint64_t poolsize = 0;
    for (const auto &o: objects) {
        auto n = (inn::Neuron*)o;
        poolsize += n->getReceptorsCount() * n->getSynapsesCount();
    }
    std::cout << poolsize << std::endl;

    Worker -> poolsize = poolsize;
    Worker -> objects = objects;
    Worker -> event = new inn::Event;
    Worker -> cv.notify_one();
}

void inn::ComputeBackendOpenCL::doWaitTarget() {
    while (!Worker->done.load()) {
        ((inn::Event*)Worker->event) -> doWaitTimed(100);
    }
}

void inn::ComputeBackendOpenCL::doProcess(void *object) {
}

[[noreturn]] void inn::ComputeBackendOpenCL::tWorker(inn::WorkerInfo *worker, cl::Context context, cl::Kernel kernel, cl::CommandQueue queue) {
#ifdef INN_OPENCL_SUPPORT
    inn::Position *rpos, *spos;
    uint64_t x = 0;
    int64_t t = 0;

    while (true) {
        std::unique_lock<std::mutex> lk(worker->m);
        worker -> cv.wait(lk);
        t = 0;

        auto input = new cl_float16[worker->poolsize];
        auto output = new cl_float4[worker->poolsize];
        auto ibuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float16)*worker->poolsize);
        auto obuffer = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_float4)*worker->poolsize);
        kernel.setArg(0, obuffer);
        kernel.setArg(1, ibuffer);

        auto computesize = ((inn::Neuron*)worker->objects[0]) -> getSignalBufferSize();

        while (t < computesize) {
            x = 0;

            for (const auto &o: worker->objects) {
                auto n = (inn::Neuron*)o;

                for (int i = 0; i < n->getReceptorsCount(); i++) {
                    auto r = n -> getReceptor(i);
                    if (!r->isLocked()) rpos = r -> getPos();
                    else rpos = r -> getPosf();

                    for (int j = 0; j < n->getEntriesCount(); j++) {
                        auto e = n -> getEntry(j);

                        for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
                            auto *s = e -> getSynapse(k);
                            spos = s -> getPos();

                            if (n->getState(t) != inn::Neuron::States::Pending)
                                input[x] = {0};
                            else
                                input[x] = {static_cast<cl_float>(rpos->getPositionValue(0)),
                                            static_cast<cl_float>(rpos->getPositionValue(1)),
                                            static_cast<cl_float>(spos->getPositionValue(0)),
                                            static_cast<cl_float>(spos->getPositionValue(1)),
                                            static_cast<cl_float>(s->getGamma()),
                                            static_cast<cl_float>(e->getIn()),

                                            static_cast<cl_float>(s->getLambda()),
                                            static_cast<cl_float>(s->getk1()),
                                            static_cast<cl_float>(s->getk2()),
                                };
                            x++;
                        }
                    }
                }
            }

            queue.enqueueWriteBuffer(ibuffer, CL_TRUE, 0, sizeof(cl_float16)*worker->poolsize, input);
            queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(worker->poolsize), cl::NullRange);
            queue.finish();
            queue.enqueueReadBuffer(obuffer, CL_TRUE, 0, sizeof(cl_float4)*worker->poolsize, output);

            for (const auto &o: worker->objects) {
                auto n = (inn::Neuron*)o;
                double p = 0;

                for (int i = 0; i < n->getReceptorsCount(); i++) {
                    auto r = n -> getReceptor(i);
                    if (!r->isLocked()) rpos = r -> getPos();
                    else rpos = r -> getPosf();

                    double fisum = 0;

                    for (int j = 0; j < n->getEntriesCount(); j++) {
                        auto e = n -> getEntry(j);

                        for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
                            auto *s = e -> getSynapse(k);
                            spos = s -> getPos();
                            fisum += output[x].s2;

                            x++;
                        }
                    }

                    r -> setFi(fisum);
                }

                p /= (double)n->getReceptorsCount();
                n -> doFinalizeInput(p);
            }

            t++;
        }
        worker -> done.store(true);
        ((inn::Event*)worker->event) -> doNotifyOne();
    }
#endif
}
