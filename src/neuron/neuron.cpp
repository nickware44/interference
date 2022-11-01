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
#include "../../include/inn/system.h"

inn::Neuron::Neuron() {
    t = 0;
    Tlo = 0;
    Xm = 0;
    DimensionsCount = 0;
    Y = 0;
    NID = 0;
    Learned = false;
    State = NotProcessed;
//    ReceptorPositionComputer = nullptr;
}

inn::Neuron::Neuron(const inn::Neuron &N) {
    t = 0;
    Tlo = N.getTlo();
    Xm = N.getXm();
    DimensionsCount = N.getDimensionsCount();
    Y = 0;
    NID = 0;
    Learned = false;
    auto elabels = N.getEntries();
    for (int64_t i = 0; i < N.getEntriesCount(); i++) Entries.insert(std::make_pair(elabels[i], new Entry(*N.getEntry(i))));
    for (int64_t i = 0; i < N.getReceptorsCount(); i++) Receptors.push_back(new Receptor(*N.getReceptor(i)));
    Links = N.getLinkOutput();
    //ReceptorPositionComputer = nullptr;
    WTin = N.getWTin();
    WTout = N.getWTout();
    State = NotProcessed;
}

inn::Neuron::Neuron(unsigned int XSize, unsigned int DC, int64_t Tl, const std::vector<std::string>& InputNames) {
    t = 0;
    Tlo = Tl;
    Xm = XSize;
    DimensionsCount = DC;
    Y = 0;
    NID = 0;
    Learned = false;
    for (auto &i: InputNames) {
        auto *E = new Entry();
        Entries.insert(std::make_pair(i, E));
    }
    State = NotProcessed;
}

void inn::Neuron::doCreateNewSynapse(const std::string& EName, std::vector<double> PosVector, double k1, unsigned int Tl) {
	if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
	}
    auto nentry = Entries.find(EName);
    if (nentry != Entries.end()) {
        nentry -> second -> doAddSynapse(new inn::Position(Xm, std::move(PosVector)), Xm, k1, Tl);
    }
}

void inn::Neuron::doCreateNewReceptor(std::vector<double> PosVector) {
//    std::cout << PosVector.size() << " " << DimensionsCount << std::endl;
    if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::EX_POSITION_DIMENSIONS);
    }
    auto *R = new Receptor(new inn::Position(Xm, std::move(PosVector)), 1);
    Receptors.push_back(R);
}

void inn::Neuron::doCreateNewReceptorCluster(const std::vector<double>& PosVector, unsigned R, unsigned C) {
    double x = PosVector[0];
    double y = PosVector[1];
    double xr = x - R, yr;
    int s = 1;
    for (int i = 0; i < C; i++) {
        yr = s*sqrt(fabs(R*R-(xr-x)*(xr-x))) + y;
        doCreateNewReceptor({xr, yr, 0});
        if (xr == x + R) {
            s = -1;
        }
        xr += s*((double)R/2);
    }
}

bool inn::Neuron::doSignalSendEntry(const std::string& From, double X, const std::vector<inn::WaveDefinition>& Wv) {
    auto entry = Entries.find(From);
    entry -> second -> doIn(X, t);
    State = NotProcessed;
    for (auto &e: Entries) {
        if (!e.second->doCheckState(t)) {
//            std::cout << "In to entry of " << Name << " from " << From << " (" << t << ") - not ready" << std::endl;
            return false;
        }
    }
//    std::cout << "In to entry of " << Name << " from " << From << " (" << t << ") - ready" << std::endl;
    State = Pending;
    ComputeBackend -> doProcessNeuron((void*)this);
    return true;
}

std::pair<int64_t, double> inn::Neuron::doSignalReceive() {
    int64_t tr = t;
    State = Received;
    return std::make_pair(tr, Y);
}

double inn::Neuron::doSignalReceive(int64_t tT) {
//    if (Multithreading) return OutputSignalQ[tT];
    if (Learned && Tlo) {
        if (tT < Tlo) return 0;
        return OutputSignalQ[tT-Tlo];
    }
    return Y;
}

bool inn::Neuron::doCheckOutputSignalQ(int64_t tT) {
    return false;
}

void inn::Neuron::doCreateCheckpoint() {
    for (auto R: Receptors) R -> doSavePos();
}

void inn::Neuron::doFinalizeInput(double P) {
    Y = P;
    t++;
    State = Computed;
//    std::cout << "Object processed " << Name << std::endl;
}

void inn::Neuron::doPrepare() {
    for (auto E: Entries) E.second -> doPrepare();
}

void inn::Neuron::doFinalize() {
    for (auto E: Entries) E.second -> doFinalize();
    for (auto R: Receptors) R -> doLock();
    if (NID) std::cout << NID << " ~ " << LastWVSum << std::endl;
    Learned = true;
}

void inn::Neuron::doReset() {
    t = 0;
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
            if (Result.empty()) Result = ComputeBackend->doCompareCPFunction(CP, CPf);
            else {
                CPR = ComputeBackend->doCompareCPFunction(CP, CPf);
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
            double Rc = ComputeBackend->doCompareFunction(RPos, RPosf);
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

void inn::Neuron::doLinkOutput(const std::string& NName) {
    Links.push_back(NName);
}

void inn::Neuron::doClearOutputLinks() {
    Links.clear();
}

void inn::Neuron::doReplaceEntryName(const std::string& Original, const std::string& New) {
    auto e = Entries.find(Original);
    if (e != Entries.end()) {
        auto entry = e -> second;
        Entries.erase(e);
        Entries.insert(std::make_pair(New, entry));
    }
}

void inn::Neuron::setTime(int64_t ts) {
    t = ts;
}

void inn::Neuron::setk1(double _k1) {
    for (auto E: Entries) E.second -> setk1(_k1);
}

void inn::Neuron::setk2(double _k2) {
    for (auto E: Entries) E.second -> setk2(_k2);
}

void inn::Neuron::setk3(double _k3) {
    for (auto R: Receptors) R -> setk3(_k3);
}

void inn::Neuron::setNID(int _NID) {
    NID = _NID;
}

void inn::Neuron::setName(const std::string& NName) {
    Name = NName;
}

std::vector<std::string> inn::Neuron::getWaitingEntries() {
    std::vector<std::string> waiting;
    for (auto &e: Entries) {
//        std::cout << e.first << " " << e.second->doCheckState(t) << std::endl;
        if (!e.second->doCheckState(t)) waiting.push_back(e.first);
    }
    return waiting;
}

std::vector<std::string> inn::Neuron::getLinkOutput() const {
    return Links;
}

std::vector<std::string> inn::Neuron::getEntries() const {
    std::vector<std::string> elist;
    for (auto &e: Entries) {
        elist.push_back(e.first);
    }
    return elist;
}

inn::Neuron::Entry* inn::Neuron::getEntry(int64_t EID) const {
    if (EID >= Entries.size()) {
        throw inn::Error(inn::EX_NEURON_ENTRIES);
    }
    auto it = Entries.begin();
    std::advance(it, EID);
    return it->second;
}

inn::Neuron::Receptor* inn::Neuron::getReceptor(int64_t RID) const {
    if (RID >= Receptors.size()) {
        throw inn::Error(inn::EX_NEURON_RECEPTORS);
    }
    return Receptors[RID];
}

int64_t inn::Neuron::getEntriesCount() const {
    return Entries.size();
}

unsigned int inn::Neuron::getSynapsesCount() const {
    unsigned int SSum = 0;
    for (auto E: Entries) SSum += E.second -> getSynapsesCount();
    return SSum;
}

int64_t inn::Neuron::getReceptorsCount() const {
    return Receptors.size();
}

int64_t inn::Neuron::getTime() const {
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

int64_t inn::Neuron::getTlo() const {
    return Tlo;
}

int inn::Neuron::getNID() const {
    return NID;
}

std::string inn::Neuron::getName() {
    return Name;
}

int inn::Neuron::getState() const {
    return State;
}

inn::Neuron::~Neuron() {
    for (const auto& E: Entries) delete E.second;
    for (auto R: Receptors) delete R;
}
