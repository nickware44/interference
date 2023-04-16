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

//    std::cout << std::endl;
//    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
//    std::cout << "Using device  : " << default_device.getInfo<CL_DEVICE_NAME>() << " (CU: " << default_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << ")" << std::endl;
//    std::cout << "Driver version: " << default_device.getInfo<CL_DRIVER_VERSION>() << std::endl;

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

                   // check if we need to compute this pair by flag
                   if (input[id].s9 == 0) output[id] = (float4)(0, 0, 0, -1);
                   else {
                       // vector length
                       float d = 0;
                       d += (input[id].s0-input[id].s2)*(input[id].s0-input[id].s2);
                       d += (input[id].s1-input[id].s3)*(input[id].s1-input[id].s3);
                       d = sqrt(d);

                       float ngamma = input[id].s4 + (input[id].s7*input[id].s5-input[id].s4/input[id].s8);

                       float e = input[id].s6 * exp(-input[id].s6*d);
                       float fi = ngamma * e;
                       float dfi = (ngamma-input[id].s4) * e;
                       float nx = 0, ny = 0;
                       if (dfi > 0) {
                           float nposd = sqrt(dfi) / d;
                           nx = fabs(input[id].s0-input[id].s2) * nposd;
                           ny = fabs(input[id].s1-input[id].s3) * nposd;
                       }

                       output[id] = (float4)(nx, ny, fi, ngamma);
                   }
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
#else
    std::cerr << "The OpenCL compute backend is not supported by the current build. Rebuild interfernce library with the INN_OPENCL_SUPPORT flag." << std::endl;
#endif
}

void inn::ComputeBackendOpenCL::doRegisterHost(const std::vector<void*> &objects) {
    PoolSize = 0;
    for (const auto &o: objects) {
        auto n = (inn::Neuron*)o;
        PoolSize += n->getReceptorsCount() * n->getSynapsesCount();
    }

    input = new cl_float16[PoolSize];
    output = new cl_float4[PoolSize];
    ibuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float16)*PoolSize);
    obuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float4)*PoolSize);
    Kernel.setArg(0, obuffer);
    Kernel.setArg(1, ibuffer);

    inn::Position *rpos, *spos;
    uint64_t x = 0;

    for (const auto &o: objects) {
        auto n = (inn::Neuron*)o;

        for (int i = 0; i < n->getReceptorsCount(); i++) {
            auto r = n -> getReceptor(i);
            if (!r->isLocked()) rpos = r -> getPos();
            else rpos = r -> getPosf();

            for (int j = 0; j < n->getEntriesCount(); j++) {
                auto e = n -> getEntry(j);

                for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
                    auto s = e -> getSynapse(k);
                    spos = s -> getPos();

                    input[x] = {static_cast<cl_float>(rpos->getPositionValue(0)),
                                static_cast<cl_float>(rpos->getPositionValue(1)),
                                static_cast<cl_float>(spos->getPositionValue(0)),
                                static_cast<cl_float>(spos->getPositionValue(1)),
                                static_cast<cl_float>(s->getGamma()),
                                0,

                                static_cast<cl_float>(s->getLambda()),
                                static_cast<cl_float>(s->getk1()),
                                static_cast<cl_float>(s->getk2()),
                                0,
                    };
                    x++;
                }
            }
        }
    }
    Objects = objects;
}

void inn::ComputeBackendOpenCL::doWaitTarget() {
#ifdef INN_OPENCL_SUPPORT
    inn::Position *rpos, *spos;
    uint64_t x = 0;

    std::vector<float> eins;
    for (const auto &o: Objects) {
        auto n = (inn::Neuron*)o;
        auto state = n->getState(n->getTime());
        auto rc = n -> getReceptorsCount();
        auto ec = n -> getEntriesCount();

        eins.clear();
        if (state == inn::Neuron::States::Pending) {
            for (int j = 0; j < ec; j++) {
                auto e = n -> getEntry(j);
                eins.push_back(e->getIn());
            }
        }

        for (int i = 0; i < rc; i++) {
            for (int j = 0; j < ec; j++) {
                auto e = n -> getEntry(j);
                auto esc = e -> getSynapsesCount();

                for (unsigned int k = 0; k < esc; k++) {
                    if (state != inn::Neuron::States::Pending)
                        input[x].s9 = 0;
                    else {
                        input[x].s5 = static_cast<cl_float>(eins[j]);
                        input[x].s9 = 1;
                    }
                    x++;
                }
            }
        }
    }

    Queue.enqueueWriteBuffer(ibuffer, CL_TRUE, 0, sizeof(cl_float16)*PoolSize, input);
    Queue.enqueueNDRangeKernel(Kernel, cl::NullRange, cl::NDRange(PoolSize), cl::NullRange);
    Queue.finish();
    Queue.enqueueReadBuffer(obuffer, CL_TRUE, 0, sizeof(cl_float4)*PoolSize, output);

    x = 0;
    std::vector<float> nr;

    for (const auto &o: Objects) {
        auto n = (inn::Neuron*)o;
        auto rc = n -> getReceptorsCount();
        auto sc = n -> getSynapsesCount();
        auto ec = n -> getEntriesCount();
        if (output[x].s3 == -1) {
            x += rc * sc;
            continue;
        }

        auto xm = n -> getXm();
        auto dcount = n -> getDimensionsCount();
        auto rpr = new inn::Position(xm, {0, 0, 0});
        auto drpos = new inn::Position(xm, dcount);
        float p = 0;

        for (int i = 0; i < rc; i++) {
            auto r = n -> getReceptor(i);
            if (!r->isLocked()) rpos = r -> getPos();
            else rpos = r -> getPosf();

            rpr -> setPosition(rpos);

            float nrx = 0, nry = 0;
            float fisum = 0;

            auto xstart = x;
            for (int j = 0; j < ec; j++) {
                auto e = n -> getEntry(j);
                auto esc = e -> getSynapsesCount();

                for (int k = 0; k < esc; k++) {
                    nrx += output[x].s0;
                    nry += output[x].s1;
                    fisum += output[x].s2;
                    input[x].s4 = output[x].s3;
                    if (!i) {
                        e -> getSynapse(k) -> setGamma(output[x].s3);
                    }
                    x++;
                }
            }

            x = xstart;
            for (int j = 0; j < sc; j++) {
                input[x].s0 = input[x].s0+nrx;
                input[x].s1 = input[x].s1+nry;
                x++;
            }

            nr = {nrx, nry, 0};
            drpos -> setPosition(nr);
            r -> setFi(fisum);
            r -> setPos(drpos);
            p += inn::Computer::getReceptorInfluenceValue(r->doCheckActive(), r->getdFi(), rpos, rpr);
            r -> doUpdateSensitivityValue();
        }

        p /= (float)n->getReceptorsCount();
        n -> doFinalizeInput(p);
    }
#endif
}

void inn::ComputeBackendOpenCL::doProcess(void *object) {
}

void inn::ComputeBackendOpenCL::doUnregisterHost() {
    delete input;
    delete output;
}
