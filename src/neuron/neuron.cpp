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
    OutputSignal = new double;
    OutputSignalSize = 1;
    OutputSignalPointer = 0;
    NID = 0;
    Learned = false;
//    ReceptorPositionComputer = nullptr;
}

inn::Neuron::Neuron(const inn::Neuron &N) {
    t = 0;
    Tlo = N.getTlo();
    Xm = N.getXm();
    OutputSignal = new double;
    OutputSignalSize = 1;
    OutputSignalPointer = 0;
    DimensionsCount = N.getDimensionsCount();
    NID = 0;
    Learned = false;
    auto elabels = N.getEntries();
    for (int64_t i = 0; i < N.getEntriesCount(); i++) Entries.insert(std::make_pair(elabels[i], new Entry(*N.getEntry(i))));
    for (int64_t i = 0; i < N.getReceptorsCount(); i++) Receptors.push_back(new Receptor(*N.getReceptor(i)));
    Links = N.getLinkOutput();
}

inn::Neuron::Neuron(unsigned int XSize, unsigned int DC, int64_t Tl, const std::vector<std::string>& InputNames) {
    t = 0;
    Tlo = Tl;
    Xm = XSize;
    DimensionsCount = DC;
    OutputSignal = new double;
    OutputSignalSize = 1;
    OutputSignalPointer = 0;
    NID = 0;
    Learned = false;
    for (auto &i: InputNames) {
        auto *E = new Entry();
        Entries.insert(std::make_pair(i, E));
    }
}

/**
 * Create new synapse.
 * @param EName Entry name to connect synapse.
 * @param PosVector Synapse position.
 * @param k1 Neurotransmitter intensity value.
 * @param Tl Reserved. Must be 0.
 * @param NT Neurotransmitter type.
 */
void inn::Neuron::doCreateNewSynapse(const std::string& EName, std::vector<double> PosVector, double k1, int64_t Tl, int NT) {
	if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
	}
    auto nentry = Entries.find(EName);
    if (nentry != Entries.end()) {
        nentry -> second -> doAddSynapse(new inn::Position(Xm, std::move(PosVector)), Xm, k1, Tl, NT);
    }
}

/**
 * Create synapse cluster.
 * @param PosVector Position of center of synapse cluster,
 * @param R Cluster radius.
 */
void inn::Neuron::doCreateNewSynapseCluster(const std::vector<double>& PosVector, unsigned R, double k1, int64_t Tl, int NT) {
    double x = PosVector[0];
    double y = PosVector[1];

    double dfi = 360. / Entries.size();
    double fi = 0;
    double xr, yr;
    for (auto &ne: Entries) {
        xr = x + R * cos(fi/180*M_PI);
        yr = y + R * sin(fi/180*M_PI);
        doCreateNewSynapse(ne.first, {xr, yr, 0}, k1, Tl, NT);
        fi += dfi;
    }
}

/**
 * Create new receptor.
 * @param PosVector Start position of receptor.
 */
void inn::Neuron::doCreateNewReceptor(std::vector<double> PosVector) {
//    std::cout << PosVector.size() << " " << DimensionsCount << std::endl;
    if (PosVector.size() != DimensionsCount) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    auto *R = new Receptor(new inn::Position(Xm, std::move(PosVector)), 1);
    Receptors.push_back(R);
}

/**
 * Create receptor cluster.
 * @param PosVector Position of center of receptor cluster,
 * @param R Cluster radius.
 * @param C Count of receptors in cluster.
 */
void inn::Neuron::doCreateNewReceptorCluster(const std::vector<double>& PosVector, unsigned R, unsigned C) {
    double x = PosVector[0];
    double y = PosVector[1];
    double dfi = 360. / C;
    double fi = 0;
    double xr, yr;
    for (int i = 0; i < C; i++) {
        xr = x + R * cos(fi/180*M_PI);
        yr = y + R * sin(fi/180*M_PI);
        doCreateNewReceptor({xr, yr, 0});
        fi += dfi;
    }
}

bool inn::Neuron::doSignalSendEntry(const std::string& From, double X, int64_t tn) {
    auto entry = Entries.find(From);
    entry -> second -> doIn(X, tn);
    for (auto &e: Entries) {
        if (!e.second->doCheckState(tn)) {
//            std::cout << "In to entry of " << Name << " from " << e.first << " (" << tn << ") - not ready" << std::endl;
            return false;
        }
    }
//    std::cout << "In to entry of " << Name << " from " << From << " value " << X <<  " (" << tn << ") - ready" << std::endl;
    inn::System::getComputeBackend() -> doProcess((void*)this);
    return true;
}

/**
 *
 * @param tT
 * @return
 */
std::pair<int64_t, double> inn::Neuron::doSignalReceive(int64_t tT) {
    auto tlocal = t.load();
    if (tT == -1) tT = tlocal - 1;
    auto d = tlocal - tT;
    if (d > 0 && OutputSignalPointer-d >= 0) {
        return std::make_pair(tT, OutputSignal[OutputSignalPointer-d]);
    } else {
        std::cerr << "[" << Name << "] Output for time " << tT << " is not ready yet" << std::endl;
        return std::make_pair(tT, 0);
    }
}

void inn::Neuron::doCreateCheckpoint() {
    for (auto R: Receptors) R -> doSavePos();
}

void inn::Neuron::doFinalizeInput(double P) {
    if (OutputSignalPointer >= OutputSignalSize) OutputSignalPointer = 0;
    OutputSignal[OutputSignalPointer] = P;
    OutputSignalPointer++;
    t.store(t.load()+1);
//    std::cout << "Object processed " << Name << std::endl;
}

/**
 * Prepare synapses for new signal.
 */
void inn::Neuron::doPrepare() {
    for (auto E: Entries) E.second -> doPrepare();
}

void inn::Neuron::doFinalize() {
    for (auto E: Entries) E.second -> doFinalize();
    for (auto R: Receptors) R -> doLock();
    Learned = true;
}

/**
 * Reset neuron state. During the reset, the neuron parameters (time, receptors, synapses) will be reset to the default state.
 */
void inn::Neuron::doReset() {
    t = 0;
    for (auto R: Receptors) {
        R -> doReset();
    }
    doPrepare();
}

std::vector<double> inn::Neuron::doCompareCheckpoints() {
    std::vector<double> Result, CPR;
    std::vector<inn::Position*> CP, CPf;
    for (auto R: Receptors) {
        if (R->isLocked()) {
            CP = R -> getCP();
            CPf = R -> getCPf();
            if (Result.empty()) Result = inn::Computer::doCompareCPFunction(CP, CPf);
            else {
                CPR = inn::Computer::doCompareCPFunction(CP, CPf);
                for (int j = 0; j < Result.size(); j++) Result[j] += CPR[j];
            }
        }
    }
    for (auto R: Result) R /= Receptors.size();
    return Result;
}

/**
 * Compare neuron patterns (learning and recognition patterns).
 * @return Pattern difference value.
 */
inn::Neuron::PatternDefinition inn::Neuron::doComparePattern() const {
    inn::Position *RPos, *RPosf;
    double Result = 0;
    double ResultL = 0;
    for (auto R: Receptors) {
        if (R->isLocked()) {
            RPos = R -> getPos();
            RPosf = R -> getPosf();
//            std::cout << Name << " " << RPos->getPositionValue(0) << ", " <<  RPos->getPositionValue(1) << ", " <<  RPos->getPositionValue(2) << std::endl;
//            std::cout << RPosf->getPositionValue(0) << ", " <<  RPosf->getPositionValue(1) << ", " <<  RPosf->getPositionValue(2) << std::endl;
            double Rc = inn::Computer::doCompareFunction(RPos, RPosf);
            double Lc = fabs(R->getL()-R->getLf());
//            std::cout << R->getL() << std::endl;
//            std::cout << R->getLf() << std::endl;
            Result += Rc;
            ResultL += Lc;
        }
    }
    Result /= Receptors.size();
    ResultL /= Receptors.size();
    return inn::Neuron::PatternDefinition(Result, ResultL);
}

void inn::Neuron::doLinkOutput(const std::string& NName) {
    Links.push_back(NName);
}

void inn::Neuron::doClearOutputLinks() {
    Links.clear();
}

/**
 * Relink neuron by replacing entry name.
 * @param Original Name of entry to rename.
 * @param New New name of entry.
 */
void inn::Neuron::doReplaceEntryName(const std::string& Original, const std::string& New) {
    auto e = Entries.find(Original);
    if (e != Entries.end()) {
        auto entry = e -> second;
        Entries.erase(e);
        Entries.insert(std::make_pair(New, entry));
    }
}

void inn::Neuron::doReserveSignalBuffer(int64_t L) {
    delete [] OutputSignal;
    OutputSignal = new double[L];
    OutputSignalSize = L;
    OutputSignalPointer = 0;
    for (auto &E: Entries) {
        E.second -> doReserveSignalBuffer(L);
    }
}

/**
 * Set time.
 * @param ts Time.
 */
void inn::Neuron::setTime(int64_t ts) {
    t.store(ts);
}

/**
 * Set neurotransmitter intensity for all synapses.
 * @param _k1
 */
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

/**
 * Set neuron to `learned` state
 * @param LearnedFlag `Learned` state flag.
 */
void inn::Neuron::setLearned(bool LearnedFlag) {
    Learned = LearnedFlag;

    if (Learned)
        for (auto R: Receptors) R -> doLock();
    else
        for (auto R: Receptors) R -> doUnlock();
}

/**
 * Check if neuron is in `learned` state.
 * @return Neuron state.
 */
bool inn::Neuron::isLearned() const {
    return Learned;
}

std::vector<std::string> inn::Neuron::getWaitingEntries() {
    std::vector<std::string> waiting;
    for (auto &e: Entries) {
        if (!e.second->doCheckState(t.load())) waiting.push_back(e.first);
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
        throw inn::Error(inn::Error::EX_NEURON_ENTRIES);
    }
    auto it = Entries.begin();
    std::advance(it, EID);
    return it->second;
}

/**
 * Get receptor by index.
 * @param RID Receptor index.
 * @return inn::Neuron::Receptor object pointer.
 */
inn::Neuron::Receptor* inn::Neuron::getReceptor(int64_t RID) const {
    if (RID >= Receptors.size()) {
        throw inn::Error(inn::Error::EX_NEURON_RECEPTORS);
    }
    return Receptors[RID];
}

/**
 * Get count of neuron entries.
 * @return Entry count.
 */
int64_t inn::Neuron::getEntriesCount() const {
    return Entries.size();
}

/**
 * Get count of neuron synapses.
 * @return Synapse count.
 */
unsigned int inn::Neuron::getSynapsesCount() const {
    unsigned int SSum = 0;
    for (const auto& E: Entries) SSum += E.second -> getSynapsesCount();
    return SSum;
}

/**
 * Get count of neuron receptors.
 * @return Receptor count.
 */
int64_t inn::Neuron::getReceptorsCount() const {
    return Receptors.size();
}

/**
 * Get current time.
 * @return Time value.
 */
int64_t inn::Neuron::getTime() const {
    return t.load();
}

/**
 * Get neuron space size.
 * @return Neuron space size value.
 */
unsigned int inn::Neuron::getXm() const {
    return Xm;
}

/**
 * Get count of neuron dimensions.
 * @return Count of dimensions.
 */
unsigned int inn::Neuron::getDimensionsCount() const {
    return DimensionsCount;
}

int64_t inn::Neuron::getTlo() const {
    return Tlo;
}

int inn::Neuron::getNID() const {
    return NID;
}

/**
 * Get neuron name.
 * @return Neuron name.
 */
std::string inn::Neuron::getName() {
    return Name;
}

int64_t inn::Neuron::getSignalBufferSize() const {
    return OutputSignalSize;
}

/**
 * Get current state of neuron.
 * @return Neuron state.
 */
int inn::Neuron::getState(int64_t tT) const {
    for (const auto &e: Entries) {
        if (!e.second->doCheckState(tT)) {
            return States::NotProcessed;
        }
    }
    if (tT >= t.load()) return States::Pending;
    return States::Computed;
}

inn::Neuron::~Neuron() {
    for (const auto& E: Entries) delete E.second;
    for (auto R: Receptors) delete R;
}
