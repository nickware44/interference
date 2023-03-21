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

int CurrentComputeBackend = -1, VerbosityLevel = 1;
int ComputeBackendParameter = 0;
bool SynchronizationNeeded;
inn::Computer *ComputeBackend;

void inn::System::setComputeBackend(int Backend, int Parameter) {
    CurrentComputeBackend = Backend;
    delete ComputeBackend;

    switch (CurrentComputeBackend) {
        case inn::System::ComputeBackends::Default:
            SynchronizationNeeded = false;
            ComputeBackend = new inn::ComputeBackendDefault();
            Parameter = 0;
            break;
        case inn::System::ComputeBackends::Multithread:
            SynchronizationNeeded = true;
            ComputeBackend = new inn::ComputeBackendMultithread(Parameter?Parameter:INN_MULTITHREAD_DEFAULT_NUM);
            break;
    }
    ComputeBackendParameter = Parameter;
}

inn::Computer* inn::System::getComputeBackend() {
    return ComputeBackend;
}

int inn::System::getComputeBackendKind() {
    return CurrentComputeBackend;
}

bool inn::System::isSynchronizationNeeded() {
    return SynchronizationNeeded;
}

void inn::System::setVerbosityLevel(int VL) {
    VerbosityLevel = VL;
}

int inn::System::getVerbosityLevel() {
    return VerbosityLevel;
}

int inn::System::getComputeBackendParameter() {
    return ComputeBackendParameter;
}

bool inn::Event::doWaitTimed(int T) {
    auto rTimeout = std::chrono::milliseconds(T);
    bool bTimeout = false;
    bool bRet;
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    while (!m_bEvent && !bTimeout) {
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
