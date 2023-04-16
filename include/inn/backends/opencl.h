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

#include "../computer.h"
#include <thread>
#include <condition_variable>
#include <atomic>

#ifdef INN_OPENCL_SUPPORT
    #include <CL/cl.hpp>
#endif

namespace inn {
    class ComputeBackendOpenCL : public Computer {
    private:
#ifdef INN_OPENCL_SUPPORT
        cl::Context Context;
        cl::Kernel Kernel;
        cl::CommandQueue Queue;
        cl::Buffer InputBuffer;
        cl::Buffer OutputBuffer;

        cl_float16 *input;
        cl_float4 *output;
        cl::Buffer ibuffer;
        cl::Buffer obuffer;
#endif
        uint64_t PoolSize;
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
