/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     06.04.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <inn/backends/opencl.h>
#include <inn/neuron.h>
#include <inn/system.h>

#define KERNEL(name, ...) std::string name = #__VA_ARGS__

inn::ComputeBackendOpenCL::ComputeBackendOpenCL() {
#ifdef INDK_OPENCL_SUPPORT
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
    int status;

//    std::cout << std::endl;
//    std::cout << "Using platform: " << default_platform.getInfo<CL_PLATFORM_NAME>() << std::endl;
//    std::cout << "Using device  : " << default_device.getInfo<CL_DEVICE_NAME>() << " (CU: " << default_device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << ")" << std::endl;
//    std::cout << "Driver version: " << default_device.getInfo<CL_DRIVER_VERSION>() << std::endl;

    KERNEL(kernel_code_pairs,
           __kernel void inn_kernel_pairs(__global float16 *pairs, __global float2 *inputs) {
                   int id = get_global_id(0);
                   // pairs.s0 - receptor x
                   // pairs.s1 - receptor y
                   // pairs.s2 - synapse x              (const)
                   // pairs.s3 - synapse y              (const)

                   // pairs.s4 - synapse gamma value
                   // pairs.s5 - input index            (const)

                   // pairs.s6 - lambda                 (const)
                   // pairs.s7 - k1                     (const)
                   // pairs.s8 - k2                     (const)

                   // pairs.s9 - reserved
                   // pairs.sA - reserved
                   // pairs.sB - reserved

                   // check if we need to compute this pair by flag
                   bool run = inputs[(int)pairs[id].s5].s0;
                   if (run) {
                       float in = inputs[(int)pairs[id].s5].s1;

                       // vector length
                       float d = 0;
                       d += (pairs[id].s0-pairs[id].s2)*(pairs[id].s0-pairs[id].s2);
                       d += (pairs[id].s1-pairs[id].s3)*(pairs[id].s1-pairs[id].s3);
                       d = sqrt(d);

                       float ngamma = pairs[id].s4 + (pairs[id].s7*in-pairs[id].s4/pairs[id].s8);

                       float e = pairs[id].s6 * exp(-pairs[id].s6*d);
                       float fi = ngamma * e;
                       float dfi = (ngamma-pairs[id].s4) * e;
                       float nx = 0, ny = 0;
                       if (dfi > 0 && d > 0) {
                           float nposd = sqrt(dfi) / d;
                           nx = (pairs[id].s0-pairs[id].s2) * nposd;
                           ny = (pairs[id].s1-pairs[id].s3) * nposd;
                       }

                       // update gamma value
                       pairs[id].s4 = ngamma;

                       pairs[id].s9 = nx;
                       pairs[id].sA = ny;
                       pairs[id].sB = fi;
                   };
           }
    );

    KERNEL(kernel_code_receptors,
           __kernel void inn_kernel_receptors(__global float8 *receptors,  __global float16 *pairs, __global float2 *inputs) {
                   int id = get_global_id(0);
                   // receptors.s0 - left pairs range edge           (const)
                   // receptors.s1 - right pairs range edge          (const)
                   // receptors.s2 - input index                     (const)

                   // receptors.s3 - receptor sensitivity
                   // receptors.s4 - neurotransmitter level value

                   // receptors.s5 - k3                              (const)

                   // receptors.s6 - reserved

                   bool run = inputs[(int)receptors[id].s2].s0;
                   if (run) {
                       float drx = 0, dry = 0, fisum = 0;

                       for (int i = receptors[id].s0; i < receptors[id].s1; i++) {
                           drx += pairs[i].s9;
                           dry += pairs[i].sA;
                           fisum += pairs[i].sB;
                       }

                       for (int i = receptors[id].s0; i < receptors[id].s1; i++) {
                           pairs[i].s0 += drx;
                           pairs[i].s1 += dry;
                       }

                       float dfisum = fisum - receptors[id].s4;
                       receptors[id].s4 = fisum;

                       float d = drx*drx + dry*dry;
                       d = sqrt(d);

                       float p = 0;
                       if (d > 0 && fisum > receptors[id].s3) p = d;

                       if (dfisum > 0 && fisum >= receptors[id].s3) receptors[id].s3 += dfisum;
                       else receptors[id].s3 = receptors[id].s3 / (receptors[id].s5*receptors[id].s3+1);

                       receptors[id].s6 = p;
                   };
           }
    );

    KERNEL(kernel_code_neurons,
           __kernel void inn_kernel_neurons(__global float3 *neurons,  __global float8 *receptors, __global float2 *inputs, __global float *outputs) {
                   int id = get_global_id(0);
                   // neurons.s0 - left receptors range edge           (const)
                   // neurons.s1 - right receptors range edge          (const)
                   // neurons.s2 - input index                         (const)

                   bool run = inputs[(int)neurons[id].s2].s0;
                   if (run) {
                       float p = 0;
                       int rcount = neurons[id].s1 - neurons[id].s0;

                       for (int i = neurons[id].s0; i < neurons[id].s1; i++) {
                           p += receptors[i].s6;
                       }
                       p /= (float)rcount;

                       outputs[id] = p;
                   };
           }
    );

    Context = cl::Context(CL_DEVICE_TYPE_ALL);

    cl::Program pairs(Context, {kernel_code_pairs.c_str(),kernel_code_pairs.length()+1});
    if ((status = pairs.build({default_device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << pairs.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        return;
    }

    cl::Program receptors(Context, {kernel_code_receptors.c_str(),kernel_code_receptors.length()+1});
    if ((status = receptors.build({default_device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << receptors.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        return;
    }

    cl::Program neurons(Context, {kernel_code_neurons.c_str(),kernel_code_neurons.length()+1});
    if ((status = neurons.build({default_device})) != CL_SUCCESS) {
        std::cerr << "Error building: status " << status << " " << neurons.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        return;
    }

    KernelPairs = cl::Kernel(pairs, "inn_kernel_pairs");
    KernelReceptors = cl::Kernel(receptors, "inn_kernel_receptors");
    KernelNeurons = cl::Kernel(neurons, "inn_kernel_neurons");
    Queue = cl::CommandQueue(Context,default_device);
#endif
}

void inn::ComputeBackendOpenCL::doRegisterHost(const std::vector<void*> &objects) {
    PairPoolSize = 0;
    ReceptorPoolSize = 0;
    InputPoolSize = 0;
    NeuronPoolSize = objects.size();
    for (const auto &o: objects) {
        auto n = (inn::Neuron*)o;
        PairPoolSize += n->getReceptorsCount() * n->getSynapsesCount();
        ReceptorPoolSize += n->getReceptorsCount();
        InputPoolSize += n->getEntriesCount();
    }
#ifdef INDK_OPENCL_SUPPORT
    PairsInfo = new cl_float16[PairPoolSize];
    ReceptorsInfo = new cl_float8[ReceptorPoolSize];
    NeuronsInfo = new cl_float3[NeuronPoolSize];
    Inputs = new cl_float2[InputPoolSize];
    Outputs = new cl_float[NeuronPoolSize];

    PairsBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float16)*PairPoolSize);
    ReceptorsBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float8)*ReceptorPoolSize);
    NeuronsBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float3)*NeuronPoolSize);
    InputsBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float2)*InputPoolSize);
    OutputsBuffer = cl::Buffer(Context, CL_MEM_READ_WRITE, sizeof(cl_float)*NeuronPoolSize);

    KernelPairs.setArg(0, PairsBuffer);
    KernelPairs.setArg(1, InputsBuffer);

    KernelReceptors.setArg(0, ReceptorsBuffer);
    KernelReceptors.setArg(1, PairsBuffer);
    KernelReceptors.setArg(2, InputsBuffer);

    KernelNeurons.setArg(0, NeuronsBuffer);
    KernelNeurons.setArg(1, ReceptorsBuffer);
    KernelNeurons.setArg(2, InputsBuffer);
    KernelNeurons.setArg(3, OutputsBuffer);

    inn::Position *rpos, *spos;
    uint64_t px = 0, pxstart;
    uint64_t rx = 0, rxstart;
    uint64_t ex = 0, exstart;

    for (uint64_t ni = 0; ni < objects.size(); ni++) {
        auto n = (inn::Neuron*)objects[ni];

        std::vector<std::pair<std::string, uint64_t>> emap;
        auto we = n -> getWaitingEntries();

        rxstart = rx;
        exstart = ex;

        for (int i = 0; i < n->getReceptorsCount(); i++) {
            pxstart = px;

            auto r = n -> getReceptor(i);
            if (!r->isLocked()) rpos = r -> getPos();
            else rpos = r -> getPosf();

            ex = exstart;
            for (int j = 0; j < n->getEntriesCount(); j++) {
                auto e = n -> getEntry(j);

                for (unsigned int k = 0; k < e->getSynapsesCount(); k++) {
                    auto s = e -> getSynapse(k);
                    spos = s -> getPos();

                    PairsInfo[px] = {
                            static_cast<cl_float>(rpos->getPositionValue(0)),
                            static_cast<cl_float>(rpos->getPositionValue(1)),
                            static_cast<cl_float>(spos->getPositionValue(0)),
                            static_cast<cl_float>(spos->getPositionValue(1)),
                            static_cast<cl_float>(s->getGamma()),
                            static_cast<cl_float>(ex),

                            static_cast<cl_float>(s->getLambda()),
                            static_cast<cl_float>(s->getk1()),
                            static_cast<cl_float>(s->getk2()),
                    };
                    px++;
                }
                //emap.emplace_back(we[j], px);
                if (!i) {
                    Inputs[ex] = {
                            static_cast<cl_float>(0),
                            static_cast<cl_float>(0),
                    };
                }
                ex++;
            }
            ReceptorsInfo[rx] = {
                    static_cast<cl_float>(pxstart),
                    static_cast<cl_float>(px),
                    static_cast<cl_float>(exstart),
                    static_cast<cl_float>(r->getRs()),
                    static_cast<cl_float>(r->getFi()),
                    static_cast<cl_float>(r->getk3()),
            };
            rx++;
        }
        NeuronsInfo[ni] = {
                static_cast<cl_float>(rxstart),
                static_cast<cl_float>(rx),
                static_cast<cl_float>(exstart),
        };
    }

    Objects = objects;

    Queue.enqueueWriteBuffer(PairsBuffer, CL_TRUE, 0, sizeof(cl_float16)*PairPoolSize, PairsInfo);
    Queue.enqueueWriteBuffer(ReceptorsBuffer, CL_TRUE, 0, sizeof(cl_float8)*ReceptorPoolSize, ReceptorsInfo);
    Queue.enqueueWriteBuffer(NeuronsBuffer, CL_TRUE, 0, sizeof(cl_float3)*NeuronPoolSize, NeuronsInfo);
    Queue.enqueueWriteBuffer(OutputsBuffer, CL_TRUE, 0, sizeof(cl_float)*NeuronPoolSize, Outputs);

#endif
}

void inn::ComputeBackendOpenCL::doWaitTarget() {
#ifdef INDK_OPENCL_SUPPORT
    uint64_t x = 0;
    for (const auto &o: Objects) {
        auto n = (inn::Neuron*)o;
        auto state = n->getState(n->getTime());
        auto ec = n -> getEntriesCount();

        for (int j = 0; j < ec; j++) {
            auto e = n -> getEntry(j);
            if (state == inn::Neuron::States::Pending) {
                Inputs[x] = {
                        static_cast<cl_float>(1),
                        static_cast<cl_float>(e->getIn()),
                };
            } else {
                Inputs[x] = {
                        static_cast<cl_float>(0),
                        static_cast<cl_float>(0),
                };
            }
            x++;
        }
    }

    Queue.enqueueWriteBuffer(InputsBuffer, CL_TRUE, 0, sizeof(cl_float2)*InputPoolSize, Inputs);

    Queue.enqueueNDRangeKernel(KernelPairs, cl::NullRange, cl::NDRange(PairPoolSize), cl::NullRange);
    Queue.finish();
    Queue.enqueueNDRangeKernel(KernelReceptors, cl::NullRange, cl::NDRange(ReceptorPoolSize), cl::NullRange);
    Queue.finish();
    Queue.enqueueNDRangeKernel(KernelNeurons, cl::NullRange, cl::NDRange(NeuronPoolSize), cl::NullRange);
    Queue.finish();

    Queue.enqueueReadBuffer(OutputsBuffer, CL_TRUE, 0, sizeof(cl_float)*NeuronPoolSize, Outputs);

    for (int ni = 0; ni < Objects.size(); ni++) {
        if (Inputs[(int)NeuronsInfo[ni].s2].s0 == 0) {
            continue;
        }
        auto n = (inn::Neuron*)Objects[ni];
        n -> doFinalizeInput(Outputs[ni]);
    }
#endif
}

void inn::ComputeBackendOpenCL::doProcess(void *object) {
}

void inn::ComputeBackendOpenCL::doUnregisterHost() {
#ifdef INDK_OPENCL_SUPPORT
    Queue.enqueueReadBuffer(PairsBuffer, CL_TRUE, 0, sizeof(cl_float16)*PairPoolSize, PairsInfo);
    Queue.enqueueReadBuffer(ReceptorsBuffer, CL_TRUE, 0, sizeof(cl_float8)*ReceptorPoolSize, ReceptorsInfo);

    uint64_t rx = 0;

    for (auto &o : Objects) {
        auto n = (inn::Neuron*)o;
        inn::Position npos(0, n->getDimensionsCount());
        auto rc = n -> getReceptorsCount();

        npos.setXm(n->getXm());
        npos.setDimensionsCount(n->getDimensionsCount());

        for (int i = 0; i < rc; i++) {
            auto r = n -> getReceptor(i);

            float nrx = PairsInfo[(int)ReceptorsInfo[rx].s0].s0;
            float nry = PairsInfo[(int)ReceptorsInfo[rx].s0].s1;

            npos.setPosition({nrx, nry, 0});
            r -> setPos(&npos);
            r -> setRs(ReceptorsInfo[rx].s3);
            r -> setFi(ReceptorsInfo[rx].s4);
            rx++;
        }
    }

    delete [] PairsInfo;
    delete [] ReceptorsInfo;
    delete [] NeuronsInfo;
#endif
}
