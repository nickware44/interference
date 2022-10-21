/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/backends/multithread.h"
#include "../../include/inn/neuron.h"
#include "../../include/inn/system.h"

std::vector<std::queue<void*>> DataQueues;
std::vector<inn::Event*> Events;

inn::ComputeBackendMultithread::ComputeBackendMultithread(int WorkersCount) {
    LastWorker = 0;
    for (int i = 0; i < WorkersCount; i++) {
        Events.emplace_back(new inn::Event());
        Workers.emplace_back(tWorker, i);
        DataQueues.emplace_back();
    }
}

void inn::ComputeBackendMultithread::doProcessNeuron(void* Object) {
    if (LastWorker >= Workers.size()) LastWorker = 0;
    DataQueues[LastWorker].push(Object);
    Events[LastWorker] -> doNotifyOne();
    LastWorker++;
//    std::cout << "Pushed object to data queue " << ((inn::Neuron*)Object)->getName()
//        << " " << ((inn::Neuron*)Object)->getTime() << std::endl;
}

[[noreturn]] void inn::ComputeBackendMultithread::tWorker(int n) {
    while (true) {
        if (DataQueues[n].empty()) {
            Events[n] -> doWait();
        }
        auto N = (inn::Neuron*)DataQueues[n].front();
        DataQueues[n].pop();

        double FiSum, D, P = 0;
        auto Xm = N -> getXm();
        auto DimensionsCount = N -> getDimensionsCount();
        auto RPr = new inn::Position(Xm, {0, 0, 0});

        auto dRPos = new inn::Position(Xm, DimensionsCount);
        auto nRPos = new inn::Position(Xm, DimensionsCount);

        inn::Position *RPos;

        for (int i = 0; i < N->getReceptorsCount(); i++) {
            auto R = N->getReceptor(i);
            if (!R->isLocked()) RPos = R -> getPos();
            else RPos = R -> getPosf();
            RPr -> setPosition(RPos);
            inn::Position *SPos;
            std::pair<double, double> FiValues;
            FiSum = 0;
            dRPos -> doZeroPosition();

            for (int j = 0; j < N->getEntriesCount(); j++) {
                auto E = N -> getEntry(j);

                E -> doProcess();
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
            P += inn::Computer::getReceptorInfluenceValue(R->doCheckActive(), R->getdFi(), RPos, RPr);
            R -> doUpdateSensitivityValue();
        }
        P /= (double)N->getReceptorsCount();

        //std::cout << "From Thread ID : " << std::this_thread::get_id() << " num: " << n << ", t: " << N->getTime() << std::endl;
        N -> doFinalizeInput(P);
        inn::doNeuralNetSync();
    }
}
