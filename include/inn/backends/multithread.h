/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_MULTITHREAD_H
#define INTERFERENCE_MULTITHREAD_H

#include <thread>
#include "../computer.h"

#define INN_MULTITHREAD_DEFAULT_NUM 2

namespace inn {
    class ComputeBackendMultithread : public Computer {
    private:
        std::vector<std::thread> Workers;
        unsigned int LastWorker;

        [[noreturn]] static void tWorker(int);
    public:
        explicit ComputeBackendMultithread(int);
        void doProcessNeuron(void*) override;
    };
}

#endif //INTERFERENCE_MULTITHREAD_H
