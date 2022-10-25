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
    bool doNeuralNetSyncWait();
    void doNeuralNetSync();
    bool isSynchronizationNeeded();

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

    template<typename Data>
    class Queue {
    public:
        Queue() = default;
        ~Queue()= default;
        void doPush(Data);
        void doPop();
        Data getFront();
        long getSize();
        bool isEmpty();
    private:
        std::queue<Data> Q;
        std::mutex m_oMutex;
        std::condition_variable m_oConditionVariable;
    };

    template<typename Data>
    void Queue<Data>::doPush(Data D) {
        if (inn::isSynchronizationNeeded()) std::unique_lock<std::mutex> lock(m_oMutex);
        Q.push(D);
    }

    template<typename Data>
    void Queue<Data>::doPop() {
        if (inn::isSynchronizationNeeded()) std::unique_lock<std::mutex> lock(m_oMutex);
        Q.pop();
    }

    template<typename Data>
    Data Queue<Data>::getFront() {
        if (inn::isSynchronizationNeeded()) std::unique_lock<std::mutex> lock(m_oMutex);
        return Q.front();
    }

    template<typename Data>
    bool Queue<Data>::isEmpty() {
        if (inn::isSynchronizationNeeded()) std::unique_lock<std::mutex> lock(m_oMutex);
        return Q.empty();
    }

    template<typename Data>
    long Queue<Data>::getSize() {
        if (inn::isSynchronizationNeeded()) std::lock_guard<std::mutex> lock(m_oMutex);
        return Q.size();
    }

    typedef enum {Default,
                  Multithread,
                  OpenCLGPU} ComputeBackends;


}

#endif //INTERFERENCE_SYSTEM_H
