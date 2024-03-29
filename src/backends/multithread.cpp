/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     23.02.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/backends/multithread.h>
#include <indk/neuron.h>
#include <indk/system.h>

indk::ComputeBackendMultithread::ComputeBackendMultithread(int WC) {
    WorkerCount = WC;
    while (Workers.size() < WorkerCount) {
        auto w = new indk::Worker;
        w -> thread = std::thread(tWorker, (void*)w);
        Workers.emplace_back(w);
    }
}

// TODO: handling of cases when computing of multiple neural networks goes in parallel
void indk::ComputeBackendMultithread::doRegisterHost(const std::vector<void*>& objects) {
    int last = 0;
    for (const auto &o: objects) {
        if (last >= Workers.size()) last = 0;
        Workers[last] -> objects.push_back(o);
        Workers[last] -> event = new indk::Event;
        Workers[last] -> done = false;
        last++;
    }
    for (const auto &w: Workers) {
        w -> cv.notify_one();
    }
}

void indk::ComputeBackendMultithread::doWaitTarget() {
    for (const auto &w: Workers) {
        while (!w->done.load()) {
            ((indk::Event*)w->event) -> doWaitTimed(100);
        }
    }
}

void indk::ComputeBackendMultithread::doProcess(void* object) {
}

[[noreturn]] void indk::ComputeBackendMultithread::tWorker(void* object) {
    auto worker = (indk::Worker*)object;

    while (true) {
        std::unique_lock<std::mutex> lk(worker->m);
        worker -> cv.wait(lk);

        auto zPos = new indk::Position(0, 3);
        auto dRPos = new indk::Position(0, 3);
        auto nRPos = new indk::Position(0, 3);

        std::vector<int64_t> times;
        int tdone = 0;
        uint64_t size = worker -> objects.size();
        while (tdone < size) {
            for (uint64_t n = 0; n < size; n++) {
                if (n >= times.size()) times.push_back(0);
                auto N = (indk::Neuron*)worker -> objects[n];
                auto t = times[n];
                auto ComputeSize = N -> getSignalBufferSize();

                if (t >= ComputeSize) continue;

                if (N->getState(t) != indk::Neuron::States::Pending) {
                    continue;
                }

                float FiSum, D, P = 0;
                auto Xm = N -> getXm();
                auto DimensionsCount = N -> getDimensionsCount();

                zPos -> setXm(Xm);
                zPos -> setDimensionsCount(DimensionsCount);
                dRPos -> setXm(Xm);
                dRPos -> setDimensionsCount(DimensionsCount);
                nRPos -> setXm(Xm);
                nRPos -> setDimensionsCount(DimensionsCount);

                indk::Position *RPos;

                for (int j = 0; j < N->getEntriesCount(); j++) {
                    auto E = N -> getEntry(j);
                    E -> doProcess();
                }

                for (int i = 0; i < N->getReceptorsCount(); i++) {
                    auto R = N->getReceptor(i);
                    if (!R->isLocked()) RPos = R -> getPos();
                    else RPos = R -> getPosf();
                    indk::Position *SPos;
                    std::pair<float, float> FiValues;
                    FiSum = 0;
                    dRPos -> doZeroPosition();

                    for (int j = 0; j < N->getEntriesCount(); j++) {
                        auto E = N -> getEntry(j);

                        for (unsigned int k = 0; k < E->getSynapsesCount(); k++) {
                            auto *S = E -> getSynapse(k);
                            SPos = S -> getPos();
                            D = SPos -> getDistanceFrom(RPos);
                            FiValues = indk::Computer::getFiFunctionValue(S->getLambda(), S->getGamma(), S->getdGamma(), D);
                            if (FiValues.second > 0) {
                                indk::Computer::getNewPosition(nRPos, RPos, SPos, indk::Computer::getFiVectorLength(FiValues.second), D);
                                dRPos -> doAdd(nRPos);
                            }
                            FiSum += FiValues.first;
                        }
                    }

                    R -> setFi(FiSum);
                    R -> doUpdatePos(dRPos);
                    P += indk::Computer::getReceptorInfluenceValue(R->doCheckActive(), R->getdFi(), dRPos, zPos);
                    R -> doUpdateSensitivityValue();
                }
                P /= (float)N->getReceptorsCount();

                N -> doFinalizeInput(P);
                t++;
                times[n] = t;
                if (t >= ComputeSize) {
                    tdone++;
                }
            }
            size = worker -> objects.size();
        }

        delete zPos;
        delete nRPos;
        delete dRPos;

        worker -> done.store(true);
        ((indk::Event*)worker->event) -> doNotifyOne();
    }
}

void indk::ComputeBackendMultithread::doUnregisterHost() {
    for (const auto &w: Workers) {
        w -> objects.clear();
        delete (indk::Event*)w->event;
    }
}
