/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     23.02.23
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/backends/multithread.h"
#include "../../include/inn/neuron.h"
#include "../../include/inn/system.h"

inn::ComputeBackendMultithread::ComputeBackendMultithread(int WC) {
    WorkerCount = WC;
    LastWorker = 0;
    while (Workers.size() < WorkerCount) {
        auto w = new inn::Worker;
        w -> thread = std::thread(tWorker, (void*)w);
        Workers.emplace_back(w);
    }
}

// TODO: handling of cases when computing of multiple neural networks goes in parallel
void inn::ComputeBackendMultithread::doRegisterHost(const std::vector<void*>& objects) {
    int last = 0;
    for (const auto &o: objects) {
        if (last >= Workers.size()) last = 0;
        Workers[last] -> objects.push_back(o);
        Workers[last] -> event = new inn::Event;
        Workers[last] -> done = false;
        last++;
    }
    for (const auto &w: Workers) {
        w -> cv.notify_one();
    }
}

void inn::ComputeBackendMultithread::doWaitTarget() {
    for (const auto &w: Workers) {
        while (!w->done.load()) {
            ((inn::Event*)w->event) -> doWaitTimed(100);
        }
    }
}

void inn::ComputeBackendMultithread::doProcess(void* object) {
}

[[noreturn]] void inn::ComputeBackendMultithread::tWorker(void* object) {
    auto worker = (inn::Worker*)object;

    while (true) {
        std::unique_lock<std::mutex> lk(worker->m);
        worker -> cv.wait(lk);

        std::vector<int64_t> times;
        int tdone = 0;
        uint64_t size = worker -> objects.size();
        while (tdone < size) {
            for (uint64_t n = 0; n < size; n++) {
                if (n >= times.size()) times.push_back(0);
                auto N = (inn::Neuron*)worker -> objects[n];
                auto t = times[n];
                auto ComputeSize = N -> getSignalBufferSize();

                if (t >= ComputeSize) continue;

                if (N->getState(t) != inn::Neuron::States::Pending) {
                    continue;
                }

                float FiSum, D, P = 0;
                auto Xm = N -> getXm();
                auto DimensionsCount = N -> getDimensionsCount();
                auto RPr = new inn::Position(Xm, DimensionsCount);

                auto dRPos = new inn::Position(Xm, DimensionsCount);
                auto nRPos = new inn::Position(Xm, DimensionsCount);

                inn::Position *RPos;

                for (int j = 0; j < N->getEntriesCount(); j++) {
                    auto E = N -> getEntry(j);
                    E -> doProcess();
                }

                for (int i = 0; i < N->getReceptorsCount(); i++) {
                    auto R = N->getReceptor(i);
                    if (!R->isLocked()) RPos = R -> getPos();
                    else RPos = R -> getPosf();
                    inn::Position *SPos;
                    std::pair<float, float> FiValues;
                    FiSum = 0;
                    dRPos -> doZeroPosition();

                    for (int j = 0; j < N->getEntriesCount(); j++) {
                        auto E = N -> getEntry(j);

                        for (unsigned int k = 0; k < E->getSynapsesCount(); k++) {
                            auto *S = E -> getSynapse(k);
                            SPos = S -> getPos();
                            D = SPos -> getDistanceFrom(RPos);
                            FiValues = inn::Computer::getFiFunctionValue(S->getLambda(), S->getGamma(), S->getdGamma(), D);
                            if (FiValues.second > 0) {
                                inn::Computer::getNewPosition(nRPos, RPos, SPos, inn::Computer::getFiVectorLength(FiValues.second), D);
                                dRPos -> doAdd(nRPos);
                            }
                            FiSum += FiValues.first;
                        }
                    }

                    R -> setFi(FiSum);
                    R -> setPos(dRPos);
                    P += inn::Computer::getReceptorInfluenceValue(R->doCheckActive(), R->getdFi(), dRPos, RPr);
                    R -> doUpdateSensitivityValue();
                }
                P /= (float)N->getReceptorsCount();

                N -> doFinalizeInput(P);
                t++;
                times[n] = t;
                if (t >= ComputeSize) {
                    tdone++;
                }
                delete nRPos;
                delete dRPos;
            }
            size = worker -> objects.size();
        }
        worker -> done.store(true);
        ((inn::Event*)worker->event) -> doNotifyOne();
    }
}

void inn::ComputeBackendMultithread::doUnregisterHost() {
    for (const auto &w: Workers) {
        w -> objects.clear();
        delete (inn::Event*)w->event;
    }
}
