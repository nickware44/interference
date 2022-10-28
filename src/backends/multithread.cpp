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

typedef struct data {
    std::mutex m;
    std::condition_variable cv;
    inn::Queue<void*> q;
} DataLine;

std::vector<DataLine*> DataLines;

inn::ComputeBackendMultithread::ComputeBackendMultithread(int WorkersCount) {
    LastWorker = 0;
    for (int i = 0; i < WorkersCount; i++) {
        auto d = new DataLine;
        DataLines.emplace_back(d);
        Workers.emplace_back(tWorker, i);
    }
}

void inn::ComputeBackendMultithread::doProcessNeuron(void* Object) {
    if (LastWorker >= Workers.size()) LastWorker = 0;
    //std::lock_guard<std::mutex> GLock(Lock);
//    std::cout << "Got neuron " << Object << std::endl;
    //std::shared_ptr<void*> threadMsg = std::make_shared<void*>(Object);
    std::unique_lock<std::mutex> lk(DataLines[LastWorker]->m);
    DataLines[LastWorker]->q.doPush(Object);

//    std::cout << "Loaded neuron " << g << std::endl;

    DataLines[LastWorker]->cv.notify_one();
    //Events[LastWorker] -> doNotifyOne();
    LastWorker++;
//    std::cout << "Pushed object to data queue " << ((inn::Neuron*)Object)->getName()
//        << " " << ((inn::Neuron*)Object)->getTime() << std::endl;
}

[[noreturn]] void inn::ComputeBackendMultithread::tWorker(int n) {
    auto dRPos = new inn::Position(500, 3);
    auto nRPos = new inn::Position(500, 3);
    while (true) {
        std::unique_lock<std::mutex> lk(DataLines[n]->m);
        while (DataLines[n]->q.isEmpty()) {
            DataLines[n]->cv.wait(lk);
        }

        if (DataLines[n]->q.isEmpty()) {
            continue;
        }

        auto N = (inn::Neuron*)DataLines[n]->q.getFront();

        DataLines[n]->q.doPop();

        double FiSum, D, P = 0;
        auto Xm = N -> getXm();
        auto DimensionsCount = N -> getDimensionsCount();
        auto RPr = new inn::Position(Xm, {0, 0, 0});

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
        //std::lock_guard<std::mutex> GLock(Lock);
        //inn::doNeuralNetSync();
    }
}
