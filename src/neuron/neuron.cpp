/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/neuron.cpp
// Purpose:     Neuron main class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuron.h>
#include <indk/error.h>
#include <indk/system.h>
#include <algorithm>

indk::Neuron::Neuron() {
    t = 0;
    Tlo = 0;
    Xm = 0;
    DimensionsCount = 0;
    OutputSignal = new float;
    OutputSignalSize = 1;
    OutputSignalPointer = 0;
    NID = 0;
    OutputMode = 0;
    Learned = false;
//    ReceptorPositionComputer = nullptr;
}

indk::Neuron::Neuron(const indk::Neuron &N) {
    t = 0;
    Tlo = N.getTlo();
    Xm = N.getXm();
    OutputSignal = new float;
    OutputSignalSize = 1;
    OutputSignalPointer = 0;
    DimensionsCount = N.getDimensionsCount();
    NID = 0;
    OutputMode = 0;
    Learned = false;
    auto elabels = N.getEntries();
    for (int64_t i = 0; i < N.getEntriesCount(); i++) Entries.emplace_back(elabels[i], new Entry(*N.getEntry(i)));
    for (int64_t i = 0; i < N.getReceptorsCount(); i++) Receptors.push_back(new Receptor(*N.getReceptor(i)));
    Links = N.getLinkOutput();
}

indk::Neuron::Neuron(unsigned int XSize, unsigned int DC, int64_t Tl, const std::vector<std::string>& InputNames) {
    t = 0;
    Tlo = Tl;
    Xm = XSize;
    DimensionsCount = DC;
    OutputSignal = new float;
    OutputSignalSize = 1;
    OutputSignalPointer = 0;
    NID = 0;
    OutputMode = 0;
    Learned = false;
    for (auto &i: InputNames) {
        auto *E = new Entry();
        Entries.emplace_back(i, E);
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
void indk::Neuron::doCreateNewSynapse(const std::string& EName, std::vector<float> PosVector, float k1, int64_t Tl, int NT) {
	if (PosVector.size() != DimensionsCount) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
	}
    for (const auto &e: Entries) {
        if (e.first == EName) {
            e.second -> doAddSynapse(new indk::Position(Xm, std::move(PosVector)), Xm, k1, Tl, NT);
            break;
        }
    }
}

/**
 * Create synapse cluster.
 * @param PosVector Position of center of synapse cluster,
 * @param R Cluster radius.
 */
void indk::Neuron::doCreateNewSynapseCluster(const std::vector<float>& PosVector, unsigned R, float k1, int64_t Tl, int NT) {
    float x = PosVector[0];
    float y = PosVector[1];

    float dfi = 360. / Entries.size();
    float fi = 0;
    float xr, yr;
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
void indk::Neuron::doCreateNewReceptor(std::vector<float> PosVector) {
//    std::cout << PosVector.size() << " " << DimensionsCount << std::endl;
    if (PosVector.size() != DimensionsCount) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    auto *R = new Receptor(new indk::Position(Xm, std::move(PosVector)), 1);
    Receptors.push_back(R);
}

/**
 * Create receptor cluster.
 * @param PosVector Position of center of receptor cluster,
 * @param R Cluster radius.
 * @param C Count of receptors in cluster.
 */
void indk::Neuron::doCreateNewReceptorCluster(const std::vector<float>& PosVector, unsigned R, unsigned C) {
    float x = PosVector[0];
    float y = PosVector[1];
    float dfi = 360. / C;
    float fi = 0;
    float xr, yr;
    for (int i = 0; i < C; i++) {
        xr = x + R * cos(fi/180*M_PI);
        yr = y + R * sin(fi/180*M_PI);
        doCreateNewReceptor({xr, yr, 0});
        fi += dfi;
    }

//    float xr = x-(C+1)*R, yr;
//    int count = C;
//    for (int i = -count; i <= count; i++) {
//        xr += R;
//        yr = y-(C+1)*R;
//        for (int j = -count; j <= count; j++) {
//            yr += R;
//            doCreateNewReceptor({xr, yr, 0});
//        }
//    }
}

bool indk::Neuron::doSignalSendEntry(const std::string& From, float X, int64_t tn) {
    for (const auto &e: Entries) {
        if (e.first == From) {
            e.second -> doIn(X, tn);
            break;
        }
    }

    for (auto &e: Entries) {
        if (!e.second->doCheckState(tn)) {
//            std::cout << "In to entry of " << Name << " from " << e.first << " (" << tn << ") - not ready" << std::endl;
            return false;
        }
    }
//    std::cout << "In to entry of " << Name << " from " << From << " value " << X <<  " (" << tn << ") - ready" << std::endl;
    indk::System::getComputeBackend() -> doProcess((void*)this);
    return true;
}

/**
 *
 * @param tT
 * @return
 */
std::pair<int64_t, float> indk::Neuron::doSignalReceive(int64_t tT) {
    auto tlocal = t.load();
    if (tT == -1) tT = tlocal - 1;
    auto d = tlocal - tT;

    if (d > 0 && OutputSignalPointer-d >= 0) {
        if (OutputMode == 1) {
            auto patterns = doComparePattern();
            if (std::get<0>(patterns) < 10e-6) {
                return std::make_pair(tT, OutputSignal[OutputSignalPointer-d]);
            } else
                return std::make_pair(tT, 0);
        }
        return std::make_pair(tT, OutputSignal[OutputSignalPointer-d]);
    } else {
        if (indk::System::getVerbosityLevel() > 1)
            std::cerr << "[" << Name << "] Output for time " << tT << " is not ready yet" << std::endl;
        return std::make_pair(tT, 0);
    }
}

void indk::Neuron::doCreateCheckpoint() {
    for (auto R: Receptors) R -> doSavePos();
}

void indk::Neuron::doFinalizeInput(float P) {
    if (OutputSignalPointer >= OutputSignalSize) OutputSignalPointer = 0;
    OutputSignal[OutputSignalPointer] = P;
    OutputSignalPointer++;
    t.store(t.load()+1);
//    std::cout << "Object processed " << Name << std::endl;
}

/**
 * Prepare synapses for new signal.
 */
void indk::Neuron::doPrepare() {
    t = 0;
    for (auto E: Entries) E.second -> doPrepare();
    for (auto R: Receptors) R -> doPrepare();
}

void indk::Neuron::doFinalize() {
    for (auto E: Entries) E.second -> doFinalize();
    for (auto R: Receptors) R -> doLock();
    Learned = true;
}

void indk::Neuron::doCreateNewScope() {
    for (auto R: Receptors) R -> doCreateNewScope();
}

void indk::Neuron::doChangeScope(uint64_t scope) {
    for (auto R: Receptors) R -> doChangeScope(scope);
}

/**
 * Reset neuron state. During the reset, the neuron parameters (time, receptors, synapses) will be reset to the default state.
 */
void indk::Neuron::doReset() {
    Learned = false;
    for (auto R: Receptors) R -> doReset();
}

std::vector<float> indk::Neuron::doCompareCheckpoints() {
    std::vector<float> Result, CPR;
    std::vector<indk::Position*> CP, CPf;
    for (auto R: Receptors) {
        if (R->isLocked()) {
            CP = R -> getCP();
            CPf = R -> getCPf();
            if (Result.empty()) Result = indk::Computer::doCompareCPFunction(CP, CPf);
            else {
                CPR = indk::Computer::doCompareCPFunction(CP, CPf);
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
indk::Neuron::PatternDefinition indk::Neuron::doComparePattern(int ProcessingMethod) const {
    indk::Position *RPosf;
    auto ssize = Receptors[0]->getReferencePosScopes().size();
    std::vector<float> results;
    float value = 0;
    float rmin = -1;

    for (uint64_t i = 0; i < ssize; i++) results.push_back(0);

    for (auto R: Receptors) {
        auto scopes = R -> getReferencePosScopes();
        RPosf = R -> getPosf();

        for (uint64_t i = 0; i < scopes.size(); i++) {
            results[i] += indk::Computer::doCompareFunction(scopes[i], RPosf) / Receptors.size();
        }
    }

    switch (ProcessingMethod) {
        default:
        case indk::ScopeProcessingMethods::ProcessMin:
            for (auto r: results) {
                if (rmin == -1 || r < rmin) rmin = r;
            }
            value = rmin;
            break;

        case indk::ScopeProcessingMethods::ProcessAverage:
            for (auto r: results) {
                value += r;
            }
            value /= ssize;
            break;
    }

    return {value, 0};
}

void indk::Neuron::doLinkOutput(const std::string& NName) {
    Links.push_back(NName);
}

void indk::Neuron::doClearOutputLinks() {
    Links.clear();
}

void indk::Neuron::doClearEntries() {
    Entries.clear();
}

void indk::Neuron::doAddEntryName(const std::string& name) {
    auto *E = new Entry();
    Entries.emplace_back(name, E);
}

void indk::Neuron::doCopyEntry(const std::string& from, const std::string& to) {
    for (auto &e: Entries) {
        if (e.first == from) {
            auto *E = new Entry(*e.second);
            Entries.emplace_back(to, E);
            break;
        }
    }
}

/**
 * Relink neuron by replacing entry name.
 * @param Original Name of entry to rename.
 * @param New New name of entry.
 */
void indk::Neuron::doReplaceEntryName(const std::string& Original, const std::string& New) {
    for (auto &e: Entries) {
        if (e.first == Original) {
            e.first = New;
            break;
        }
    }
}

void indk::Neuron::doReserveSignalBuffer(int64_t L) {
    delete [] OutputSignal;
    OutputSignal = new float[L];
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
void indk::Neuron::setTime(int64_t ts) {
    t.store(ts);
}

void indk::Neuron::setEntries(const std::vector<std::string>& inputs) {
    for (const auto& e: Entries)
        delete e.second;

    for (const auto &i: inputs) {
        auto *E = new Entry();
        Entries.emplace_back(i, E);
    }
}

void indk::Neuron::setLambda(float _l) {
    for (auto E: Entries) E.second -> setLambda(_l);
}

/**
 * Set neurotransmitter intensity for all synapses.
 * @param _k1
 */
void indk::Neuron::setk1(float _k1) {
    for (auto E: Entries) E.second -> setk1(_k1);
}

void indk::Neuron::setk2(float _k2) {
    for (auto E: Entries) E.second -> setk2(_k2);
}

void indk::Neuron::setk3(float _k3) {
    for (auto R: Receptors) R -> setk3(_k3);
}

void indk::Neuron::setNID(int _NID) {
    NID = _NID;
}

void indk::Neuron::setName(const std::string& NName) {
    Name = NName;
}

/**
 * Set neuron to `learned` state
 * @param LearnedFlag `Learned` state flag.
 */
void indk::Neuron::setLearned(bool LearnedFlag) {
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
bool indk::Neuron::isLearned() const {
    return Learned;
}

std::vector<std::string> indk::Neuron::getWaitingEntries() {
    std::vector<std::string> waiting;
    for (auto &e: Entries) {
        if (!e.second->doCheckState(t.load())) waiting.push_back(e.first);
    }
    return waiting;
}

std::vector<std::string> indk::Neuron::getLinkOutput() const {
    return Links;
}

std::vector<std::string> indk::Neuron::getEntries() const {
    std::vector<std::string> elist;
    for (auto &e: Entries) {
        elist.push_back(e.first);
    }
    return elist;
}

indk::Neuron::Entry* indk::Neuron::getEntry(int64_t EID) const {
    if (EID < 0 || EID >= Entries.size()) {
        throw indk::Error(indk::Error::EX_NEURON_ENTRIES);
    }
    auto it = Entries.begin();
    std::advance(it, EID);
    return it->second;
}

/**
 * Get receptor by index.
 * @param RID Receptor index.
 * @return indk::Neuron::Receptor object pointer.
 */
indk::Neuron::Receptor* indk::Neuron::getReceptor(int64_t RID) const {
    if (RID < 0 || RID >= Receptors.size()) {
        throw indk::Error(indk::Error::EX_NEURON_RECEPTORS);
    }
    return Receptors[RID];
}

/**
 * Get count of neuron entries.
 * @return Entry count.
 */
int64_t indk::Neuron::getEntriesCount() const {
    return Entries.size();
}

/**
 * Get count of neuron synapses.
 * @return Synapse count.
 */
unsigned int indk::Neuron::getSynapsesCount() const {
    unsigned int SSum = 0;
    for (const auto& E: Entries) SSum += E.second -> getSynapsesCount();
    return SSum;
}

/**
 * Get count of neuron receptors.
 * @return Receptor count.
 */
int64_t indk::Neuron::getReceptorsCount() const {
    return Receptors.size();
}

/**
 * Get current time.
 * @return Time value.
 */
int64_t indk::Neuron::getTime() const {
    return t.load();
}

/**
 * Get neuron space size.
 * @return Neuron space size value.
 */
unsigned int indk::Neuron::getXm() const {
    return Xm;
}

/**
 * Get count of neuron dimensions.
 * @return Count of dimensions.
 */
unsigned int indk::Neuron::getDimensionsCount() const {
    return DimensionsCount;
}

int64_t indk::Neuron::getTlo() const {
    return Tlo;
}

int indk::Neuron::getNID() const {
    return NID;
}

/**
 * Get neuron name.
 * @return Neuron name.
 */
std::string indk::Neuron::getName() {
    return Name;
}

int64_t indk::Neuron::getSignalBufferSize() const {
    return OutputSignalSize;
}

/**
 * Get current state of neuron.
 * @return Neuron state.
 */
int indk::Neuron::getState(int64_t tT) const {
    for (const auto &e: Entries) {
        if (!e.second->doCheckState(tT)) {
            return States::NotProcessed;
        }
    }
    if (tT >= t.load()) return States::Pending;
    return States::Computed;
}

indk::Neuron::~Neuron() {
    for (const auto& E: Entries) delete E.second;
    for (auto R: Receptors) delete R;
}
