/////////////////////////////////////////////////////////////////////////////
// Name:        computer.hpp
// Purpose:     inn::Computer class template
// Author:      Nickolay Babbysh
// Created:     10.06.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_COMPUTER_H
#define INTERFERENCE_COMPUTER_H

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace inn {
    template <class T1, class T2>
    class Computer {
    private:
        std::atomic<bool> KeepWorkerAlive;
        T2 *Instance;
        bool (T2::*doBeforeWork)(unsigned long long);
        void (T2::*doWork)();
        std::thread Worker;
        std::atomic<unsigned long long> tTM;
        void tComputerWorker();
    public:
        Computer();
        Computer(T2*, bool (T2::*)(unsigned long long), void (T2::*)());
        void doWait();
        ~Computer() = default;
    };

    template <class T1, class T2>
    inn::Computer<T1, T2>::Computer() {
        KeepWorkerAlive = true;
        tTM = 0;
    }

    template <class T1, class T2>
    inn::Computer<T1, T2>::Computer(T2 *_Instance, bool (T2::*_BeforeWorkFunction)(unsigned long long),
            void (T2::*_WorkFunction)()) {
        KeepWorkerAlive = true;
        Instance = _Instance;
        doBeforeWork = _BeforeWorkFunction;
        doWork = _WorkFunction;
        Worker = std::thread(&inn::Computer<T1, T2>::tComputerWorker, this);
        tTM = 0;
    }

    template <class T1, class T2>
    void inn::Computer<T1, T2>::tComputerWorker() {
        bool DataReady;
        while (true) {
            DataReady = (Instance->*doBeforeWork)(tTM);
            if (DataReady) {
                (Instance->*doWork)();
                tTM++;
            } else std::this_thread::sleep_for(std::chrono::microseconds(1000));
            if (!KeepWorkerAlive && !DataReady) break;
        }
    }

    template<class T1, class T2>
    void inn::Computer<T1, T2>::doWait() {
        KeepWorkerAlive = false;
        if (Worker.joinable()) Worker.join();
    }
}

#endif //INTERFERENCE_COMPUTER_H
