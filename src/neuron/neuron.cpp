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
    Tlo = 0;
    Xm = 0;
    DimensionsCount = 0;
    P = 0;
    Y = 0;
    Multithreading = false;
    NID = 0;
    Learned = false;
    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);
    ReceptorPositionComputer = nullptr;
}

inn::Neuron::Neuron(const Neuron &N) {
    t = 0;
    Tlo = N.getTlo();
    Xm = N.getXm();
    DimensionsCount = N.getDimensionsCount();
    P = 0;
    Y = 0;
    Multithreading = N.isMultithreadingEnabled();
    NID = 0;
    Learned = false;
    for (unsigned long long i = 0; i < N.getEntriesCount(); i++) Entries.push_back(new Entry(*N.getEntry(i)));
    for (unsigned long long i = 0; i < N.getReceptorsCount(); i++) Receptors.push_back(new Receptor(*N.getReceptor(i)));
    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);
    ReceptorPositionComputer = nullptr;
    WTin = N.getWTin();
    WTout = N.getWTout();
    if (Tlo) OutputSignalQ.reserve(inn::Neuron::System::getOutputSignalQMaxSizeValue(Xm));
}

inn::Neuron::Neuron(unsigned int _Xm, unsigned int _DimensionsCount, unsigned long long _Tlo, inn::WaveType _WTin, inn::WaveType _WTout) {
    t = 0;
    Tlo = _Tlo;
    Xm = _Xm;
    DimensionsCount = _DimensionsCount + 1;
    P = 0;
    Y = 0;
    Multithreading = false;
    NID = 0;
    Learned = false;
    dRPos = new inn::Position(Xm, DimensionsCount);
    nRPos = new inn::Position(Xm, DimensionsCount);
    ReceptorPositionComputer = nullptr;
    WTin = _WTin;
    WTout = _WTout;
    if (Tlo) OutputSignalQ.reserve(inn::Neuron::System::getOutputSignalQMaxSizeValue(Xm));
}

void inn::Neuron::doEnableMultithreading() {
    if (!Tlo) OutputSignalQ.reserve(inn::Neuron::System::getOutputSignalQMaxSizeValue(Xm));
    Multithreading = true;
}

void inn::Neuron::doCreateNewEntries(unsigned int EC) {
    for (unsigned int i = 0; i < EC; i++) {
        auto *E = new Entry();
        Entries.push_back(E);
    }
}

void inn::Neuron::doCreateNewSynaps(unsigned int EID, std::vector<double> PosVector, unsigned int Tl) {
    PosVector.push_back(0);
	if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
	}
    Entries[EID] -> doAddSynaps(new inn::Position(Xm, std::move(PosVector)), Xm, Tl);
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
                doCreateNewReceptor({xr, yr, 5});
                if (xr == x + R) {
                    s = -1;
                }
                xr += s*(R/2);
            }
            break;
        case 1:
            yr = y - R;
            for (int i = 0; i < 36; i++) {
                doCreateNewReceptor({xr, yr, 10});
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
	double FiSum = 0, D = 0;
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
    //auto RPr = new inn::Position(Xm, {0, 0, 0});
    inn::Position *RPos;
    for (auto R: Receptors) {
        if (!R->isLocked()) RPos = R -> getPos();
        else RPos = R -> getPosf();
        //RPr -> setPosition(RPos);
        //doComputeNewPosition(R);
        //P += inn::Neuron::System::getReceptorInfluenceValue(R->doCheckActive(), R->getdFi(), RPos, RPr);
        //R -> doUpdateSensitivityValue();
    }
    P /= Receptors.size();
    if (Multithreading || Tlo) OutputSignalQ[t] = P;
    Y = P;
    t++;
}

void inn::Neuron::doSignalSendEntry(unsigned long long EID, double X, const std::vector<inn::WaveDefinition>& Wv) {
    if (EID >= Entries.size()) {
        throw inn::Error(inn::EX_NEURON_INPUT);
    }
    double WVSum = 0;
    for (auto Wvi: Wv) {
        if (Wvi.first != WTin) WVSum += Wvi.second;
        else WVSum -= Wvi.second;
    }
    //if (WVSum != 0) WVSum = exp(WVSum*3) / 3;
    LastWVSum = WVSum;
    //if (static_cast<int>(WTin) == 7 && WVSum != 0) std::cout << t << " ~ " << WVSum << std::endl;
    if (!Multithreading) Entries[EID] -> doIn(X, t, WVSum);
    else Entries[EID] -> doSendToQueue(X, t, WVSum);
}

double inn::Neuron::doSignalReceive() {
    if (Learned && Tlo) {
        if (t < Tlo) return 0;
        return OutputSignalQ[t-Tlo];
    }
    return Y;
}

double inn::Neuron::doSignalReceive(unsigned long long tT) {
    if (Multithreading) return OutputSignalQ[tT];
    if (Learned && Tlo) {
        if (tT < Tlo) return 0;
        return OutputSignalQ[tT-Tlo];
    }
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
    if (NID) std::cout << NID << " ~ " << LastWVSum << std::endl;
    Learned = true;
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

inn::Neuron::PatternDefinition inn::Neuron::doComparePattern() const {
    inn::Position *RPos, *RPosf;
    double Result = 0;
    double ResultL = 0;
    for (auto R: Receptors) {
        if (R->isLocked()) {
            RPos = R -> getPos();
            RPosf = R -> getPosf();
            //std::cout << RPos->getPositionValue(0) << ", " <<  RPos->getPositionValue(1) << ", " <<  RPos->getPositionValue(2) << std::endl;
            //std::cout << RPosf->getPositionValue(0) << ", " <<  RPosf->getPositionValue(1) << ", " <<  RPosf->getPositionValue(2) << std::endl;
            double Rc = inn::Neuron::System::doCompareFunction(RPos, RPosf);
            //double Lc = fabs(R->getL()-R->getLf());
//            std::cout << R->getL() << std::endl;
//            std::cout << R->getLf() << std::endl;
            Result += Rc;
            //ResultL += Lc;
        }
    }
    //std::cout << "=====" << std::endl;
    Result /= Receptors.size();
    //ResultL /= Receptors.size();
    //std::cout << ResultL << std::endl;
    //std::cout << "=====" << std::endl;
    return inn::Neuron::PatternDefinition(Result, ResultL);
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

void inn::Neuron::setNID(int _NID) {
    NID = _NID;
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

inn::WaveType inn::Neuron::getWTin() const {
    return WTin;
}

inn::WaveType inn::Neuron::getWTout() const {
    return WTout;
}

inn::WaveDefinition inn::Neuron::getWave() const {
    double WV = 0;
    if (WTout != inn::WaveType::NOWAVE && t > 1000) {
        auto PD = doComparePattern();
        //WV = 1 / std::get<0>(PD);
    }
    return std::pair<inn::WaveType, double>(WTout, WV);
}

unsigned long long inn::Neuron::getTlo() const {
    return Tlo;
}

int inn::Neuron::getNID() const {
    return NID;
}

inn::Neuron::~Neuron() {
    for (auto E: Entries) delete E;
    for (auto R: Receptors) delete R;
}
