/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/system.h>
#include <indk/backends/default.h>
#include <indk/backends/multithread.h>
#include <indk/backends/opencl.h>

int CurrentComputeBackend = -1, VerbosityLevel = 1;
int ComputeBackendParameter = 0;
bool SynchronizationNeeded;
indk::Computer *ComputeBackend;

void indk::System::setComputeBackend(int Backend, int Parameter) {
    if (CurrentComputeBackend != -1) delete ComputeBackend;
    CurrentComputeBackend = Backend;

    switch (CurrentComputeBackend) {
        case indk::System::ComputeBackends::Default:
            SynchronizationNeeded = false;
            ComputeBackend = new indk::ComputeBackendDefault();
            Parameter = 1;
            break;
        case indk::System::ComputeBackends::Multithread:
            SynchronizationNeeded = true;
            ComputeBackend = new indk::ComputeBackendMultithread(Parameter>1?Parameter:indk_MULTITHREAD_DEFAULT_NUM);
            break;
        case indk::System::ComputeBackends::OpenCL:
#ifdef INDK_OPENCL_SUPPORT
            SynchronizationNeeded = true;
            ComputeBackend = new indk::ComputeBackendOpenCL();
            Parameter = 1;
            break;
#else
            std::cerr << std::endl;
            std::cerr << "The OpenCL compute backend is not supported by the current build. Rebuild interfernce library with the INDK_OPENCL_SUPPORT=ON flag." << std::endl;
            CurrentComputeBackend = -1;
            return;
#endif
    }
    ComputeBackendParameter = Parameter;
}

indk::Computer* indk::System::getComputeBackend() {
    return ComputeBackend;
}

int indk::System::getComputeBackendKind() {
    return CurrentComputeBackend;
}

bool indk::System::isSynchronizationNeeded() {
    return SynchronizationNeeded;
}

void indk::System::setVerbosityLevel(int VL) {
    VerbosityLevel = VL;
}

int indk::System::getVerbosityLevel() {
    return VerbosityLevel;
}

int indk::System::getComputeBackendParameter() {
    return ComputeBackendParameter;
}

bool indk::Event::doWaitTimed(int T) {
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

bool indk::Event::doWait() {
    bool bRet;
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    m_oConditionVariable.wait(oNotifierLock);
    bRet = m_bEvent;
    m_bEvent = false;
    return bRet;
}

void indk::Event::doNotifyOne() {
    std::unique_lock<std::mutex> oNotifierLock(m_oMutex);
    m_bEvent = true;
    m_oConditionVariable.notify_one();
}
