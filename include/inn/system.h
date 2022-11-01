/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_SYSTEM_H
#define INTERFERENCE_SYSTEM_H

#include <mutex>
#include <condition_variable>
#include "computer.h"

namespace inn {
    extern inn::Computer *ComputeBackend;
    void setComputeBackend(int Backend, int Parameter = 0);
    int getComputeBackend();
    bool isSynchronizationNeeded();
    void setVerbosityLevel(int);
    int getVerbosityLevel();

    class Event {
    public:
        Event(): m_bEvent(false) {}
        ~Event() = default;
        template<typename DurationType>
        bool TimedWait(DurationType const& rTimeout);
        bool doWait();
        void doNotifyOne();
    private:
        std::mutex m_oMutex;
        std::condition_variable m_oConditionVariable;
        bool m_bEvent;
    };

    typedef enum {Default,
                  Multithread,
                  OpenCLGPU} ComputeBackends;


}

#endif //INTERFERENCE_SYSTEM_H
