/////////////////////////////////////////////////////////////////////////////
// Name:        neuralnet/link.cpp
// Purpose:     Neural net links class
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
}

inn::NeuralNet::Link::Link(inn::LinkType _LT, inn::Neuron *_LinkFromE) {
    Latency = 0;
    LT = _LT;
    LinkFromEID = -1;
    LinkFromE = _LinkFromE;
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
