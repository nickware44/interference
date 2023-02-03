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


std::vector<inn::DataLine*> DataLines;

inn::ComputeBackendMultithread::ComputeBackendMultithread(int WorkersCount) {
    LastWorker = 0;
    for (int i = 0; i < WorkersCount; i++) {
        auto d = new inn::DataLine;
        DataLines.emplace_back(d);
        Workers.emplace_back(tWorker, i);
    }
}

void inn::ComputeBackendMultithread::doProcessNeuron(void* Object) {
    std::lock_guard<std::mutex> ProcessLock(m);
    if (LastWorker >= Workers.size()) LastWorker = 0;
    std::unique_lock<std::mutex> lk(DataLines[LastWorker]->m);
    DataLines[LastWorker]->q.push(Object);
    DataLines[LastWorker]->cv.notify_one();
    LastWorker++;
}

[[noreturn]] void inn::ComputeBackendMultithread::tWorker(int n) {
    while (true) {
        std::unique_lock<std::mutex> lk(DataLines[n]->m);
        while (DataLines[n]->q.empty()) {
            DataLines[n]->cv.wait(lk);
        }

        if (DataLines[n]->q.empty()) {
            continue;
        }

        auto N = (inn::Neuron*)DataLines[n]->q.front();

        DataLines[n]->q.pop();

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
    }
}
