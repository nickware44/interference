/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/neuron.cpp
// Purpose:     Neuron main class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"
#include "../../include/inn/error.h"

inn::Neuron::Neuron() {
    t = 0;
    Xm = 0;
    DimensionsCount = 0;
    P = 0;
}

inn::Neuron::Neuron(const Neuron &N) {
    const auto NEntries = N.getEntries();
    const auto NReceptors = N.getReceptors();

    for (auto NE: NEntries) {
        auto *E = new Entry(*NE);
        Entries.push_back(E);
    }

    for (auto NR: NReceptors) {
        auto *R = new Receptor(*NR);
        Receptors.push_back(R);
    }
}

inn::Neuron::Neuron(unsigned int _Xm, unsigned int _DimensionsCount) {
    t = 0;
    Xm = _Xm;
    DimensionsCount = _DimensionsCount;
    P = 0;
}

void inn::Neuron::doCreateNewEntries(unsigned int EC) {
    for (unsigned int i = 0; i < EC; i++) {
        auto *E = new Entry();
        Entries.push_back(E);
    }
}

void inn::Neuron::doCreateNewSynaps(unsigned int EID, Position Pos, unsigned int Tl) {
    if (Pos.getDimensionsCount() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
    }
    Entries[EID] -> doAddSynaps(Pos, Tl, 0);
}

void inn::Neuron::doCreateNewSynaps(unsigned int EID, Position Pos, unsigned int Tl, unsigned int Type) {
    if (Pos.getDimensionsCount() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
    }
    Entries[EID] -> doAddSynaps(Pos, Tl, Type);
}

void inn::Neuron::doCreateNewReceptor(Position Pos) {
    if (Pos.getDimensionsCount() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
    }
    auto *R = new Receptor(Pos, 2);
    Receptors.push_back(R);
}

void inn::Neuron::doCreateNewReceptorCluster(double x, double y, double D, TopologyID TID) {
    double R = D/2, xr = x - R, yr;
    int s = 1;
    switch (TID) {
        case 0:
            for (int i = 0; i < 9; i++) {
                yr = s*sqrt(fabs(R*R-(xr-x)*(xr-x))) + y;
                doCreateNewReceptor(Position(Xm, {xr, yr}));
                if (xr == x + R) {
                    s = -1;
                }
                xr += s*(R/2);
            }
            break;
        case 1:
            yr = y - R;
            for (int i = 0; i < 36; i++) {
                doCreateNewReceptor(Position(Xm, {xr, yr}));
                xr += D / 5;
                if (xr > x + R) {
                    yr += D / 5;
                    xr = x - R;
                }
            }
            break;
        default:
            doCreateNewReceptorCluster(x, y, D, 0);
    }
}

float inn::Neuron::doSignalsSend(std::vector<double> X) {
    double FiSum, Fi, dFi;
    Position RPos, SPos;

    P = 0;
    if (X.size() != Entries.size()) {
        throw inn::Error(inn::EX_NEURON_INPUT);
    }

    for (int j = 0; j < Entries.size(); j++) Entries[j] -> doIn(X[j], t);

    for (auto R: Receptors) {
        FiSum = 0;
        Position NewPos;
        for (auto E: Entries) {
            for (unsigned int k = 0; k < E->getSynapsesCount(); k++) {
                Synaps *S = E -> getSynaps(k);
                if (R->isLocked()) RPos = R -> getPosf();
                else RPos = R -> getPos();
                SPos = S -> getPos();
                Fi = System::getFiFunctionValue(RPos, SPos, S->getLambda(), S->getGamma());
                dFi = System::getFiFunctionValue(RPos, SPos, S->getLambda(), S->getdGamma());
                if (dFi > 0) NewPos = NewPos + System::getNewPosition(RPos, SPos, System::getFiVectorLength(dFi));
                FiSum += Fi;
            }
        }
        R -> setFi(FiSum);
        R -> doUpdateSensitivityValue();
        P += R -> doCheckActive();
        R -> setPos(NewPos);
    }

    t++;

    P /= Receptors.size();
    return P;
}

bool inn::Neuron::doSignalReceive() {
    return P >= 0.5;
}

void inn::Neuron::doCreateCheckpoint() {
    for (auto R: Receptors) R -> doSavePos();
}

void inn::Neuron::doFinalize() {
    for (auto R: Receptors) R -> doLock();

    t = 0;

    for (auto E: Entries) {
        E -> doClearSignal();
        for (unsigned int j = 0; j < E->getSynapsesCount(); j++) {
            Synaps *Sk = E -> getSynaps(j);
            Sk -> doClearGamma();
        }
    }
}

std::vector<double> inn::Neuron::doCompareCheckpoints() {
    std::vector<double> Result, CPR;
    std::vector<Position> CP, CPf;

    for (auto R: Receptors) {
        if (R->isLocked()) {
            CP = R -> getCP();
            CPf = R -> getCPf();
            //unsigned long CPC = CP.size();
            if (Result.empty()) Result = System::doCompareCPFunction(CP, CPf);
            else {
                CPR = System::doCompareCPFunction(CP, CPf);
                for (int j = 0; j < Result.size(); j++) Result[j] += CPR[j];
            }
        }
    }

    for (auto R: Result) R /= Receptors.size();
    return Result;
}

double inn::Neuron::doComparePattern() {
    return doComparePattern(false);
}

double inn::Neuron::doComparePattern(bool WCP) {
    inn::Position RPos, RPosf;
    std::vector<inn::Position> CP, CPf;
    double Result = 0;

    for (auto R: Receptors) {
        if (R->isLocked()) {
            RPos = R -> getPos();
            RPosf = R -> getPosf();
            CP = R -> getCP();
            CPf = R -> getCPf();
            //unsigned long L = CP.size();
            //if (CPf.size() < L) L = CPf.size();
            double Rc = System::doCompareFunction(RPos, RPosf, R->getL(), R->getLf());
            if (WCP) Rc += System::doCompareCPFunctionD(CP, CPf);
            //Rc /= L + 1;
            Result += Rc;
        }
    }

    Result /= Receptors.size();
    return Result;
}

std::vector<inn::Neuron::Entry*> inn::Neuron::getEntries() const {
    return Entries;
}

std::vector<inn::Neuron::Receptor*> inn::Neuron::getReceptors() const {
    return Receptors;
}

unsigned long long inn::Neuron::getEntriesCount() {
    return Entries.size();
}

unsigned int inn::Neuron::getSynapsesCount() {
    unsigned int SSum = 0;
    for (auto E: Entries) SSum += E -> getSynapsesCount();
    return SSum;
}

unsigned long long inn::Neuron::getReceptorsCount() {
    return Receptors.size();
}

unsigned int inn::Neuron::getDimensionsCount() {
    return DimensionsCount;
}
