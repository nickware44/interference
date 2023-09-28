/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/synaps.cpp
// Purpose:     Neuron synaps class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuron.h>
#include <indk/system.h>

indk::Neuron::Synapse::Synapse() {
    SPos = new indk::Position();
    ok1 = 0;
    ok2 = 0;
    k1 = 0;
    k2 = 0;
    Lambda = 0;
    Tl = 0;
    Gamma = 0;
    dGamma = 0;
    QCounter = -1;
    QSize = 0;
}

indk::Neuron::Synapse::Synapse(const Synapse &S) {
    SPos = S.getPos();
    ok1 = S.getk1();
    ok2 = S.getk2();
    k1 = ok1;
    k2 = ok2;
    Lambda = S.getLambda();
    Tl = S.getTl();
    Gamma = S.getGamma();
    dGamma = S.getdGamma();
    QCounter = -1;
    QSize = 0;
}

indk::Neuron::Synapse::Synapse(indk::Position *_SPos, float _k1, float _Lambda, int64_t _Tl, int NT) {
    SPos = _SPos;
    ok1 = _k1;
    ok2 = ok1 * 1000;
    k1 = ok1;
	k2 = ok2;
    Lambda = _Lambda;
    Tl = _Tl;
    Gamma = 0;
    dGamma = 0;
    QCounter = -1;
    QSize = 0;
    NeurotransmitterType = NT;
}

void indk::Neuron::Synapse::doIn(float X) {
    float nGamma = indk::Computer::getGammaFunctionValue(Gamma, k1, k2, X);
    dGamma = nGamma - Gamma;
    Gamma = nGamma;
}

void indk::Neuron::Synapse::doSendToQueue(float X, float WVSum) {
//    float GammaLast = !QSize ? 0 : GammaQ[QSize-1];
//    float nGamma = ComputeBackend->getGammaFunctionValue(GammaLast, k1, k2, X);
//    GammaQ[QSize] = nGamma;
//    QSize++;
}

bool indk::Neuron::Synapse::doInFromQueue(int64_t tT) {
    if (tT >= QSize) return false;
    if (tT == QCounter) return true;
    float nGamma = GammaQ[tT];
    dGamma = nGamma - Gamma;
    Gamma = nGamma;
    QCounter = tT;
    return true;
}

void indk::Neuron::Synapse::doPrepare() {
    ok1 = k1;
    ok2 = k2;
}

void indk::Neuron::Synapse::doReset() {
    Gamma = 0;
    dGamma = 0;
    QCounter = 0;
    QSize = 0;
    GammaQ.clear();
    k1 = ok1;
    k2 = ok2;
}

void indk::Neuron::Synapse::setGamma(float gamma) {
    dGamma = gamma - Gamma;
    Gamma = gamma;
}

void indk::Neuron::Synapse::setk1(float _k1) {
    k1 = _k1;
}

void indk::Neuron::Synapse::setk2(float _k2) {
    k2 = _k2;
}

void indk::Neuron::Synapse::setLambda(float L) {
    Lambda = L;
}

indk::Position* indk::Neuron::Synapse::getPos() const {
    return SPos;
}

float indk::Neuron::Synapse::getk1() const {
    return k1;
}

float indk::Neuron::Synapse::getk2() const {
    return k2;
}

float indk::Neuron::Synapse::getLambda() const {
    return Lambda;
}

int64_t indk::Neuron::Synapse::getTl() const {
    return Tl;
}

float indk::Neuron::Synapse::getGamma() const {
    return Gamma;
}

float indk::Neuron::Synapse::getdGamma() const {
    return dGamma;
}

int64_t indk::Neuron::Synapse::getQSize() {
    return QSize;
}

int indk::Neuron::Synapse::getNeurotransmitterType() const {
    return NeurotransmitterType;
}
