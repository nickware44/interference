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
#include <thread>
#include <condition_variable>
#include <atomic>

#ifdef INN_OPENCL_SUPPORT
    #include <CL/cl2.hpp>
#endif

namespace inn {
    typedef struct winfo {
        std::vector<void*> objects;
        uint64_t poolsize;
        std::thread thread;
        std::mutex m;
        std::condition_variable cv;
        void *event;
        std::atomic<bool> done;
    } WorkerInfo;

    class ComputeBackendOpenCL : public Computer {
    private:
#ifdef INN_OPENCL_SUPPORT
        cl::Context Context;
        cl::Kernel Kernel;
        cl::CommandQueue Queue;
        cl::Buffer InputBuffer;
        cl::Buffer OutputBuffer;
#endif

        inn::WorkerInfo *Worker;

        [[noreturn]] static void tWorker(inn::WorkerInfo*, cl::Context, cl::Kernel, cl::CommandQueue);
    public:
        ComputeBackendOpenCL();
        void doRegisterHost(const std::vector<void*>&) override;
        void doWaitTarget() override;
        void doProcess(void*) override;
    };
}

#endif //INTERFERENCE_OPENCL_H
