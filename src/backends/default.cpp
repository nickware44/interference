/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"
#include "../../include/inn/backends/default.h"

inn::ComputeBackendDefault::ComputeBackendDefault() {
    dRPos = nullptr;
    nRPos = nullptr;
}

void inn::ComputeBackendDefault::doRegisterHost(const std::vector<void*>&) {
}

void inn::ComputeBackendDefault::doWaitTarget() {
}

void inn::ComputeBackendDefault::doProcess(void* Object) {
    auto N = (inn::Neuron*)Object;
    float FiSum, D, P = 0;

    auto Xm = N -> getXm();
    auto DimensionsCount = N -> getDimensionsCount();
    auto RPr = new inn::Position(Xm, {0, 0, 0});

    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);

    inn::Position *RPos;

    for (int j = 0; j < N->getEntriesCount(); j++) {
        auto E = N -> getEntry(j);
        E -> doProcess();
    }

    for (int i = 0; i < N->getReceptorsCount(); i++) {
        auto R = N->getReceptor(i);
        if (!R->isLocked()) RPos = R -> getPos();
        else RPos = R -> getPosf();
        RPr -> setPosition(RPos);
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
        P += inn::Computer::getReceptorInfluenceValue(R->doCheckActive(), R->getdFi(), RPos, RPr);
        R -> doUpdateSensitivityValue();
    }
    P /= (float)N->getReceptorsCount();
    delete dRPos;
    delete nRPos;

    N -> doFinalizeInput(P);
}

void inn::ComputeBackendDefault::doUnregisterHost() {
}
