/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/entry.cpp
// Purpose:     Neuron entry class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuron.h>
#include <indk/system.h>

indk::Neuron::Entry::Entry() {
    t = 0;
    tm = -1;
    SignalPointer = 0;
    Signal = new float;
    SignalSize = 1;
}

indk::Neuron::Entry::Entry(const Entry &E) {
    for (int64_t i = 0; i < E.getSynapsesCount(); i++) {
        auto *S = new Synapse(*E.getSynapse(i));
        Synapses.push_back(S);
    }
    t = 0;
    tm = -1;
    Signal = new float;
    SignalSize = 1;
    SignalPointer = 0;
}

bool indk::Neuron::Entry::doCheckState(int64_t tn) const {
//    std::cout << "check state " << tm << " " << tn << std::endl;
    return tm >= tn;
}

void indk::Neuron::Entry::doAddSynapse(indk::Position *SPos, unsigned int Xm, float k1, int64_t Tl, int NT) {
	auto *S = new Synapse(SPos, k1, indk::Computer::getLambdaValue(Xm), Tl, NT);
    Synapses.push_back(S);
}

void indk::Neuron::Entry::doIn(float Xt, int64_t tn) {
    if (SignalPointer >= SignalSize) SignalPointer = 0;
    Signal[SignalPointer] = Xt;
    SignalPointer++;
    tm = tn;
//    std::cout << "In entry " << tm << " " << t << " " << SignalPointer << " " << Signal[SignalPointer-1] << " " << Xt << std::endl;
}

void indk::Neuron::Entry::doProcess() {
    auto d = tm - t + 1;
//    std::cout << "Entry " << tm << " " << t << " " << d << " " << SignalPointer-d << " " << Signal[SignalPointer-d] << std::endl;
    if (d > 0 && SignalPointer-d >= 0) {
//        std::cout << SignalPointer-d << std::endl;
        for (auto S: Synapses) {
            if (t >= S->getTl()) S -> doIn(Signal[SignalPointer-d]);
            else S -> doIn(0);
        }
        t++;
    }
}

void indk::Neuron::Entry::doPrepare() {
    t = 0;
    tm = -1;
    for (auto S: Synapses) S -> doReset();
    //for (auto S: Synapses) S -> doPrepare();
}

void indk::Neuron::Entry::doFinalize() {
    //for (auto Sig: Signal)
        //for (auto S: Synapses) S -> doIn(Sig);
    for (auto S: Synapses) S -> doReset();
    //Signal.clear();
}

void indk::Neuron::Entry::doReserveSignalBuffer(uint64_t L) {
    delete [] Signal;
    Signal = new float[L];
    SignalSize = L;
    SignalPointer = 0;
}

void indk::Neuron::Entry::setk1(float _k1) {
    for (auto S: Synapses) S -> setk1(_k1);
}

void indk::Neuron::Entry::setk2(float _k2) {
    for (auto S: Synapses) S -> setk2(_k2);
}

indk::Neuron::Synapse* indk::Neuron::Entry::getSynapse(int64_t SID) const {
    return Synapses[SID];
}

int64_t indk::Neuron::Entry::getSynapsesCount() const {
    return Synapses.size();
}

float indk::Neuron::Entry::getIn() {
    auto d = tm - t + 1;
    if (d > 0 && SignalPointer-d >= 0) {
        t++;
        return Signal[SignalPointer-d];
    }
    return 0;
}

indk::Neuron::Entry::~Entry() {
    for (auto S: Synapses) delete S;
}
