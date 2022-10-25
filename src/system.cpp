/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../include/inn/system.h"
#include "../include/inn/backends/default.h"
#include "../include/inn/backends/multithread.h"

int CurrentComputeBackend;
bool SynchronizationNeeded;
inn::Computer *inn::ComputeBackend;

//std::shared_ptr<inn::Event> NNSyncEvent;
inn::Event *NNSyncEvent;

void inn::setComputeBackend(int Backend, int Parameter) {
    CurrentComputeBackend = Backend;
    delete ComputeBackend;
    delete NNSyncEvent;

    switch (CurrentComputeBackend) {
        case inn::ComputeBackends::Default:
            SynchronizationNeeded = false;
            ComputeBackend = new inn::ComputeBackendDefault();
            break;
        case inn::ComputeBackends::Multithread:
            SynchronizationNeeded = true;
            NNSyncEvent = new inn::Event();
            ComputeBackend = new inn::ComputeBackendMultithread(Parameter?Parameter:INN_MULTITHREAD_DEFAULT_NUM);
            break;
    }
}

int inn::getComputeBackend() {
    return CurrentComputeBackend;
}

bool inn::isSynchronizationNeeded() {
    return SynchronizationNeeded;
}

template<typename DurationType>
bool inn::Event::TimedWait(DurationType const& rTimeout) {
    bool bTimeout = false;
    bool bRet;
    std::unique_lock< std::mutex > oNotifierLock(m_oMutex);
    while(!m_bEvent && !bTimeout)
    {
        bTimeout = std::cv_status::timeout == m_oConditionVariable.wait_for(oNotifierLock, rTimeout);
    }
    bRet = m_bEvent;
    m_bEvent = false;
    return bRet;
}


bool inn::Event::doWait() {
    bool bRet;
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    m_oConditionVariable.wait(oNotifierLock);
    bRet = m_bEvent;
    m_bEvent = false;
    return bRet;
}

void inn::Event::doNotifyOne() {
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    m_bEvent = true;
    m_oConditionVariable.notify_one();
}

bool inn::doNeuralNetSyncWait() {
    return NNSyncEvent->doWait();
}

void inn::doNeuralNetSync() {
    NNSyncEvent -> doNotifyOne();
}

