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

std::queue<void*> DataQueue;

inn::ComputeBackendMultithread::ComputeBackendMultithread(int WorkersCount) {
    for (int i = 0; i < WorkersCount; i++) {
        Workers.emplace_back(tWorker, i);
    }
}

void inn::ComputeBackendMultithread::doProcessNeuron(void* Object) {
    DataQueue.push(Object);
}

[[noreturn]] void inn::ComputeBackendMultithread::tWorker(int n) {
    while (true) {
        if (DataQueue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        auto N = (inn::Neuron*)DataQueue.front();
        DataQueue.pop();

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

        N -> doFinalizeInput(P);
        std::cout << "From Thread ID : " << std::this_thread::get_id() << " num: " << n << std::endl;
    }
}
