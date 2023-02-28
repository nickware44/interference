/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     18.10.22
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_MULTITHREAD_H
#define INTERFERENCE_MULTITHREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>
#include <atomic>
#include "../computer.h"

#define INN_MULTITHREAD_DEFAULT_NUM 2

namespace inn {
    /// \private
    typedef struct worker {
        std::vector<void*> objects;
        std::thread thread;
        void *event;
        std::atomic<bool> done;
        std::mutex m;
        std::condition_variable cv;

    } Worker;

    class ComputeBackendMultithread : public Computer {
    private:
        std::vector<inn::Worker*> Workers;
        std::map<void*, unsigned int> ObjectTable;
        unsigned int WorkerCount, LastWorker;

        [[noreturn]] static void tWorker(void*);
    public:
        explicit ComputeBackendMultithread(int);
        void doRegisterHost(const std::vector<void*>&) override;
        void doWaitTarget() override;
        void doProcess(void*) override;
    };
}

#endif //INTERFERENCE_MULTITHREAD_H
