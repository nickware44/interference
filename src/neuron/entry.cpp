/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/entry.cpp
// Purpose:     Neuron entry class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"

inn::Neuron::Entry::Entry(const Entry &E) {
    for (unsigned long long i = 0; i < E.getSynapsesCount(); i++) {
        auto *S = new Synaps(*E.getSynaps(i));
        Synapses.push_back(S);
    }
}

void inn::Neuron::Entry::doAddSynaps(inn::Position *SPos, unsigned int Xm, unsigned int Tl, int Type) {
	auto *S = new Synaps(SPos, 0.8, inn::Neuron::System::getLambdaValue(Xm), Tl, Type);
    Synapses.push_back(S);
}

void inn::Neuron::Entry::doIn(double X, unsigned long long t) {
    //Signal.push_back(X);

    unsigned long long STl = 0;

    for (auto S: Synapses) {
        STl = S -> getTl();
        if (t >= STl) S -> doIn(X);
        else S -> doIn(0);
    }
}

void inn::Neuron::Entry::doSendToQueue(double X, unsigned long long t) {
    //Signal.push_back(X);

    unsigned long long STl = 0;

    for (auto S: Synapses) {
        STl = S -> getTl();
        if (t >= STl) S -> doSendToQueue(X);
        else S -> doSendToQueue(0);
    }
}

bool inn::Neuron::Entry::doInFromQueue(unsigned long long tT) {
    for (auto S: Synapses) if (!S->doInFromQueue(tT)) return false;
    return true;
}

void inn::Neuron::Entry::doPrepare() {
    for (auto S: Synapses) S -> doPrepare();
}

void inn::Neuron::Entry::doFinalize() {
    //for (auto Sig: Signal)
        //for (auto S: Synapses) S -> doIn(Sig);
    for (auto S: Synapses) S -> doReset();
    Signal.clear();
}

void inn::Neuron::Entry::doClearSignal() {
    Signal.clear();
}

void inn::Neuron::Entry::setk1(double _k1) {
    for (auto S: Synapses) S -> setk1(_k1);
}

void inn::Neuron::Entry::setk2(double _k2) {
    for (auto S: Synapses) S -> setk2(_k2);
}

void inn::Neuron::Entry::setNeurotransmitterType(int NTType) {
    for (auto S: Synapses) S -> setNeurotransmitterType(NTType);
}

inn::Neuron::Synaps* inn::Neuron::Entry::getSynaps(unsigned long long SID) const {
    return Synapses[SID];
}

unsigned long long inn::Neuron::Entry::getSynapsesCount() const {
    return Synapses.size();
}

inn::Neuron::Entry::~Entry() {
    for (auto S: Synapses) delete S;
}
