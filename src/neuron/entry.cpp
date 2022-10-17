/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/entry.cpp
// Purpose:     Neuron entry class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"

inn::Neuron::Entry::Entry() {
    t = -1;
}

inn::Neuron::Entry::Entry(const Entry &E) {
    for (int64_t i = 0; i < E.getSynapsesCount(); i++) {
        auto *S = new Synapse(*E.getSynapse(i));
        Synapses.push_back(S);
    }
    t = -1;
}

bool inn::Neuron::Entry::doCheckState(int64_t tn) const {
    return tn == t;
}

void inn::Neuron::Entry::doAddSynapse(inn::Position *SPos, unsigned int Xm, unsigned int Tl) {
	auto *S = new Synapse(SPos, 10.8, inn::Neuron::System::getLambdaValue(Xm), Tl);
    Synapses.push_back(S);
}

void inn::Neuron::Entry::doIn(double X, int64_t tn, double WVSum) {
    //Signal.push_back(X);

    int64_t STl;
    t = tn;

    for (auto S: Synapses) {
        STl = S -> getTl();
        if (tn >= STl) S -> doIn(X, WVSum);
        else S -> doIn(0, WVSum);
    }
}

void inn::Neuron::Entry::doSendToQueue(double X, int64_t t, double WVSum) {
    //Signal.push_back(X);

    int64_t STl = 0;

    for (auto S: Synapses) {
        STl = S -> getTl();
        if (t >= STl) S -> doSendToQueue(X, WVSum);
        else S -> doSendToQueue(0, WVSum);
    }
}

bool inn::Neuron::Entry::doInFromQueue(int64_t tT) {
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

void inn::Neuron::Entry::setWTs(inn::WaveType _WTs) {
    for (auto S: Synapses) S -> setWTs(_WTs);
}

inn::Neuron::Synapse* inn::Neuron::Entry::getSynapse(int64_t SID) const {
    return Synapses[SID];
}

int64_t inn::Neuron::Entry::getSynapsesCount() const {
    return Synapses.size();
}

inn::Neuron::Entry::~Entry() {
    for (auto S: Synapses) delete S;
}
