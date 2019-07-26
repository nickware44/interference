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
    Y = 0;
    Multithreading = false;
    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);
    ReceptorPositionComputer = nullptr;
}

inn::Neuron::Neuron(const Neuron &N) {
    t = 0;
    Xm = N.getXm();
    DimensionsCount = N.getDimensionsCount();
    P = 0;
    Y = 0;
    Multithreading = N.isMultithreadingEnabled();
    for (unsigned long long i = 0; i < N.getEntriesCount(); i++) Entries.push_back(new Entry(*N.getEntry(i)));
    for (unsigned long long i = 0; i < N.getReceptorsCount(); i++) Receptors.push_back(new Receptor(*N.getReceptor(i)));
    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);
    ReceptorPositionComputer = nullptr;
}

inn::Neuron::Neuron(unsigned int _Xm, unsigned int _DimensionsCount) {
    t = 0;
    Xm = _Xm;
    DimensionsCount = _DimensionsCount;
    P = 0;
    Y = 0;
    Multithreading = false;
    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);
    ReceptorPositionComputer = nullptr;
}

void inn::Neuron::doEnableMultithreading() {
    OutputSignalQ.reserve(inn::Neuron::System::getOutputSignalQMaxSizeValue(Xm));
    Multithreading = true;
}

void inn::Neuron::doCreateNewEntries(unsigned int EC) {
    for (unsigned int i = 0; i < EC; i++) {
        auto *E = new Entry();
        Entries.push_back(E);
    }
}

void inn::Neuron::doCreateNewSynaps(unsigned int EID, std::vector<double> PosVector, unsigned int Tl, unsigned int Type = 0) {
	if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
	}
	Entries[EID] -> doAddSynaps(new inn::Position(Xm, std::move(PosVector)), Xm, Tl, Type);
}

void inn::Neuron::doCreateNewReceptor(std::vector<double> PosVector) {
    if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
    }
    auto *R = new Receptor(new inn::Position(Xm, std::move(PosVector)), 1);
    Receptors.push_back(R);
}

void inn::Neuron::doCreateNewReceptorCluster(double x, double y, double D, TopologyID TID) {
    double R = D/2, xr = x - R, yr;
    int s = 1;
    switch (TID) {
        case 0:
            for (int i = 0; i < 9; i++) {
                yr = s*sqrt(fabs(R*R-(xr-x)*(xr-x))) + y;
                doCreateNewReceptor({xr, yr});
                if (xr == x + R) {
                    s = -1;
                }
                xr += s*(R/2);
            }
            break;
        case 1:
            yr = y - R;
            for (int i = 0; i < 36; i++) {
                doCreateNewReceptor({xr, yr});
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

bool inn::Neuron::doPrepareEntriesData(unsigned long long tT) {
    for (auto E: Entries) if (!E->doInFromQueue(tT)) return false;
    return true;
}

void inn::Neuron::doComputeNewPosition(inn::Neuron::Receptor *R) {
	double FiSum = 0, D;
	inn::Position *RPos, *SPos;
    std::pair<double, double> FiValues;
    FiSum = 0;
    if (R->isLocked()) RPos = R -> getPosf();
    else RPos = R -> getPos();
    dRPos -> doZeroPosition();
    for (auto E: Entries) {
        for (unsigned int k = 0; k < E->getSynapsesCount(); k++) {
            Synaps *S = E -> getSynaps(k);
            SPos = S -> getPos();
            D = SPos -> getDistanceFrom(RPos);
            FiValues = inn::Neuron::System::getFiFunctionValue(S->getLambda(), S->getGamma(), S->getdGamma(), D);
            if (FiValues.second > 0) {
				inn::Neuron::System::getNewPosition(nRPos, RPos, SPos, inn::Neuron::System::getFiVectorLength(FiValues.second), D);
				dRPos -> doAdd(nRPos);
            }
            FiSum += FiValues.first;
        }
    }
	R -> setFi(FiSum);
    R -> setPos(dRPos);
}

void inn::Neuron::doSignalsSend() {
    P = 0;
    auto RPr = new inn::Position(Xm, {0, 0});
    inn::Position *RPos;
    for (auto R: Receptors) {
        if (!R->isLocked()) RPos = R -> getPos();
        else RPos = R -> getPosf();
        RPr -> setPosition(RPos);
        doComputeNewPosition(R);
        P += inn::Neuron::System::getReceptorInfluenceValue(R->doCheckActive(), R->getdFi(), RPos, RPr);
        R -> doUpdateSensitivityValue();
    }
    P /= Receptors.size();
    if (Multithreading) OutputSignalQ[t] = P;
    Y = P;
    t++;
}

void inn::Neuron::doSignalSendEntry(unsigned long long EID, double X) {
    if (EID >= Entries.size()) {
        throw inn::Error(inn::EX_NEURON_INPUT);
    }
    if (!Multithreading) Entries[EID] -> doIn(X, t);
    else Entries[EID] -> doSendToQueue(X, t);
}

double inn::Neuron::doSignalReceive() {
    return Y;
}

double inn::Neuron::doSignalReceive(unsigned long long tT) {
    if (Multithreading) return OutputSignalQ[tT];
    return Y;
}

bool inn::Neuron::doCheckOutputSignalQ(unsigned long long tT) {
    return !Multithreading || tT < t;
}

void inn::Neuron::doCreateCheckpoint() {
    for (auto R: Receptors) R -> doSavePos();
}

void inn::Neuron::doPrepare() {
    for (auto E: Entries) E -> doPrepare();
    if (Multithreading)
        ReceptorPositionComputer = new inn::Computer<inn::Neuron::Receptor*, inn::Neuron>(this, &inn::Neuron::doPrepareEntriesData, &inn::Neuron::doSignalsSend);
}

void inn::Neuron::doFinalize() {
    if (Multithreading) ReceptorPositionComputer -> doWait();
    for (auto E: Entries) E -> doFinalize();
    for (auto R: Receptors) R -> doLock();
    delete ReceptorPositionComputer;
}

void inn::Neuron::doReinit() {
    t = 0;
    P = 0;
    for (auto R: Receptors) {
        R -> doReset();
        R -> doPrepare();
    }
    OutputSignalQ.clear();
    doPrepare();
}

std::vector<double> inn::Neuron::doCompareCheckpoints() {
    std::vector<double> Result, CPR;
    std::vector<inn::Position*> CP, CPf;
    for (auto R: Receptors) {
        if (R->isLocked()) {
            CP = R -> getCP();
            CPf = R -> getCPf();
            if (Result.empty()) Result = inn::Neuron::System::doCompareCPFunction(CP, CPf);
            else {
                CPR = inn::Neuron::System::doCompareCPFunction(CP, CPf);
                for (int j = 0; j < Result.size(); j++) Result[j] += CPR[j];
            }
        }
    }
    for (auto R: Result) R /= Receptors.size();
    return Result;
}

double inn::Neuron::doComparePattern() {
    inn::Position *RPos, *RPosf;
    double Result = 0;
    for (auto R: Receptors) {
        if (R->isLocked()) {
            RPos = R -> getPos();
            RPosf = R -> getPosf();
            double Rc = inn::Neuron::System::doCompareFunction(RPos, RPosf);
            Result += Rc;
        }
    }

    Result /= Receptors.size();
    return Result;
}

bool inn::Neuron::isMultithreadingEnabled() const {
    return Multithreading;
}

void inn::Neuron::setk1(double _k1) {
    for (auto E: Entries) E -> setk1(_k1);
}

void inn::Neuron::setk2(double _k2) {
    for (auto E: Entries) E -> setk2(_k2);
}

void inn::Neuron::setk3(double _k3) {
    for (auto R: Receptors) R -> setk3(_k3);
}

inn::Neuron::Entry* inn::Neuron::getEntry(unsigned long long EID) const {
    if (EID >= Entries.size()) {
        throw inn::Error(inn::EX_NEURON_ENTRIES);
    }
    return Entries[EID];
}

inn::Neuron::Receptor* inn::Neuron::getReceptor(unsigned long long RID) const {
    if (RID >= Receptors.size()) {
        throw inn::Error(inn::EX_NEURON_RECEPTORS);
    }
    return Receptors[RID];
}

unsigned long long inn::Neuron::getEntriesCount() const {
    return Entries.size();
}

unsigned int inn::Neuron::getSynapsesCount() const {
    unsigned int SSum = 0;
    for (auto E: Entries) SSum += E -> getSynapsesCount();
    return SSum;
}

unsigned long long inn::Neuron::getReceptorsCount() const {
    return Receptors.size();
}

unsigned long long inn::Neuron::getTime() const {
    return t;
}

unsigned int inn::Neuron::getXm() const {
    return Xm;
}

unsigned int inn::Neuron::getDimensionsCount() const {
    return DimensionsCount;
}

inn::Neuron::~Neuron() {
    for (auto E: Entries) delete E;
    for (auto R: Receptors) delete R;
}
