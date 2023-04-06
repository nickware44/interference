/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 06.04.23
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_OPENCL_H
#define INTERFERENCE_OPENCL_H

#include "../computer.h"

#ifdef INN_OPENCL_SUPPORT
    #include <CL/cl2.hpp>
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
        cl::Event *Event;
        cl_float16 *InputData;
        cl_float4 *OutputData;
#endif
        uint64_t PoolSize;
    public:
        ComputeBackendOpenCL();
        void doRegisterHost(const std::vector<void*>&) override;
        void doWaitTarget() override;
        void doProcess(void*) override;
    };
}

#endif //INTERFERENCE_OPENCL_H
