/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/synaps.cpp
// Purpose:     Neuron synaps class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"

inn::Neuron::Synaps::Synaps() {
    SPos = inn::Position();
    k1 = 0;
    k2 = 0;
    Lambda = 0;
    Tl = 0;
    NeurotransmitterType = 0;
    Gamma = 0;
    dGamma = 0;
}

inn::Neuron::Synaps::Synaps(const Synaps &S) {
    SPos = S.getPos();
    k1 = S.getk1();
    k2 = S.getk2();
    Lambda = S.getLambda();
    Tl = S.getTl();
    NeurotransmitterType = S.getNeurotransmitterType();
    Gamma = S.getGamma();
    dGamma = S.getdGamma();
}

inn::Neuron::Synaps::Synaps(inn::Position _SPos, unsigned long long _Tl, int _Type) {
    SPos = _SPos;
    Tl = _Tl;
    NeurotransmitterType = _Type;
    Gamma = 0;
    dGamma = 0;
}

void inn::Neuron::Synaps::doIn(double X) {
    double nGamma = inn::Neuron::System::getGammaFunctionValue(Gamma, k1, k2, X, NeurotransmitterType);
    dGamma = nGamma - Gamma;
    Gamma = nGamma;
}

void inn::Neuron::Synaps::doClearGamma() {
    Gamma = 0;
    dGamma = 0;
}

inn::Position inn::Neuron::Synaps::getPos() const {
    return SPos;
}

double inn::Neuron::Synaps::getk1() const {
    return k1;
}

double inn::Neuron::Synaps::getk2() const {
    return k2;
}

double inn::Neuron::Synaps::getLambda() const {
    return Lambda;
}

unsigned long long inn::Neuron::Synaps::getTl() const {
    return Tl;
}

int inn::Neuron::Synaps::getNeurotransmitterType() const {
    return NeurotransmitterType;
}

double inn::Neuron::Synaps::getGamma() const {
    return Gamma;
}

double inn::Neuron::Synaps::getdGamma() const {
    return dGamma;
}