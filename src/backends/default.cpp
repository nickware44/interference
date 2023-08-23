/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuron.h>
#include <indk/backends/default.h>

indk::ComputeBackendDefault::ComputeBackendDefault() {
    zPos = new indk::Position(0, 3);
    dRPos = new indk::Position(0, 3);
    nRPos = new indk::Position(0, 3);
}

void indk::ComputeBackendDefault::doRegisterHost(const std::vector<void*>&) {
}

void indk::ComputeBackendDefault::doWaitTarget() {
}

void indk::ComputeBackendDefault::doProcess(void* Object) {
    auto N = (indk::Neuron*)Object;
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
}

void indk::ComputeBackendDefault::doUnregisterHost() {
}
