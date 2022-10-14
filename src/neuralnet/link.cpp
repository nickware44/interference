/////////////////////////////////////////////////////////////////////////////
// Name:        neuralnet/link.cpp
// Purpose:     Neural net link class
// Author:      Nickolay Babbysh
// Created:     15.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuralnet.h"

inn::NeuralNet::Link::Link(inn::LinkType _LT, int _LinkFromEID) {
    Latency = 0;
    LT = _LT;
    LinkFromEID = _LinkFromEID;
    LinkFromE = nullptr;
    t = 0;
}

inn::NeuralNet::Link::Link(inn::LinkType _LT, inn::Neuron *_LinkFromE) {
    Latency = 0;
    LT = _LT;
    LinkFromEID = -1;
    LinkFromE = _LinkFromE;
    t = 0;
}

bool inn::NeuralNet::Link::doCheckSignal() {
    if (LinkFromE) {
        return LinkFromE -> doCheckOutputSignalQ(t);
    }
    return false;
}

void inn::NeuralNet::Link::doResetSignalController() {
    t = 0;
}

void inn::NeuralNet::Link::setLatency(unsigned int _Latency) {
    Latency = _Latency;
}

inn::LinkType inn::NeuralNet::Link::getLinkType() {
    return LT;
}

unsigned int inn::NeuralNet::Link::getLatency() {
    return Latency;
}

int inn::NeuralNet::Link::getLinkFromEID() {
    return LinkFromEID;
}

inn::Neuron *inn::NeuralNet::Link::getLinkFromE() {
    return LinkFromE;
}

double inn::NeuralNet::Link::getSignal() {
    if (LinkFromE) {
        return LinkFromE -> doSignalReceive(t++);
    }
    return 0;
}

int64_t inn::NeuralNet::Link::getTime() {
    return t;
}
