/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     06.04.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_OPENCL_H
#define INTERFERENCE_OPENCL_H

#include <indk/computer.h>
#include <thread>
#include <condition_variable>
#include <atomic>

#ifdef INDK_OPENCL_SUPPORT
    #include <CL/cl.hpp>
#endif

namespace indk {
    class ComputeBackendOpenCL : public Computer {
    private:
#ifdef INDK_OPENCL_SUPPORT
        cl::Context Context;
        cl::Kernel KernelPairs;
        cl::Kernel KernelReceptors;
        cl::Kernel KernelNeurons;
        cl::CommandQueue Queue;

        cl_float16 *PairsInfo;
        cl_float8 *ReceptorsInfo;
        cl_float3 *NeuronsInfo;
        cl_float2 *Inputs;
        cl_float *Outputs;

        cl::Buffer PairsBuffer;
        cl::Buffer ReceptorsBuffer;
        cl::Buffer NeuronsBuffer;
        cl::Buffer InputsBuffer;
        cl::Buffer OutputsBuffer;
#endif
        uint64_t PairPoolSize;
        uint64_t ReceptorPoolSize;
        uint64_t NeuronPoolSize;
        uint64_t InputPoolSize;
        std::vector<void*> Objects;
    public:
        ComputeBackendOpenCL();
        void doRegisterHost(const std::vector<void*>&) override;
        void doUnregisterHost() override;
        void doWaitTarget() override;
        void doProcess(void*) override;
    };
}

#endif //INTERFERENCE_OPENCL_H
