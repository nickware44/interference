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
    SPos = new inn::Position();
    ok1 = 0;
    ok2 = 0;
    k1 = 0;
    k2 = 0;
    Lambda = 0;
    Tl = 0;
    WTs = inn::WaveType::NOWAVE;
    Gamma = 0;
    dGamma = 0;
    QCounter = -1;
    QSize = 0;
}

inn::Neuron::Synaps::Synaps(const Synaps &S) {
    SPos = S.getPos();
    ok1 = S.getk1();
    ok2 = S.getk2();
    k1 = ok1;
    k2 = ok2;
    Lambda = S.getLambda();
    Tl = S.getTl();
    WTs = S.getWTs();
    Gamma = S.getGamma();
    dGamma = S.getdGamma();
    QCounter = -1;
    QSize = 0;
    GammaQ.reserve(inn::Neuron::System::getGammaQMaxSizeValue(Lambda));
}

inn::Neuron::Synaps::Synaps(inn::Position *_SPos, double _k1, double _Lambda, unsigned long long _Tl) {
    SPos = _SPos;
    ok1 = _k1;
    ok2 = ok1 * 9e-1;
    k1 = ok1;
	k2 = ok2;
    Lambda = _Lambda;
    Tl = _Tl;
    WTs = inn::WaveType::NOWAVE;
    Gamma = 0;
    dGamma = 0;
    QCounter = -1;
    QSize = 0;
    GammaQ.reserve(inn::Neuron::System::getGammaQMaxSizeValue(Lambda));
}

void inn::Neuron::Synaps::doIn(double X, double WVSum) {
    double nGamma = inn::Neuron::System::getGammaFunctionValue(Gamma, k1, k2, X, WVSum);
    dGamma = nGamma - Gamma;
    Gamma = nGamma;
}

void inn::Neuron::Synaps::doSendToQueue(double X, double WVSum) {
    double GammaLast = !QSize ? 0 : GammaQ[QSize-1];
    double nGamma = inn::Neuron::System::getGammaFunctionValue(GammaLast, k1, k2, X, WVSum);
    GammaQ[QSize] = nGamma;
    QSize++;
}

bool inn::Neuron::Synaps::doInFromQueue(unsigned long long tT) {
    if (tT >= QSize) return false;
    if (tT == QCounter) return true;
    double nGamma = GammaQ[tT];
    dGamma = nGamma - Gamma;
    Gamma = nGamma;
    QCounter = tT;
    return true;
}

void inn::Neuron::Synaps::doPrepare() {
    ok1 = k1;
    ok2 = k2;
}

void inn::Neuron::Synaps::doReset() {
    Gamma = 0;
    dGamma = 0;
    QCounter = 0;
    QSize = 0;
    GammaQ.clear();
    k1 = ok1;
    k2 = ok2;
}

void inn::Neuron::Synaps::setk1(double _k1) {
    k1 = _k1;
}

void inn::Neuron::Synaps::setk2(double _k2) {
    k2 = _k2;
}

void inn::Neuron::Synaps::setWTs(inn::WaveType _WTs) {
    WTs = _WTs;
}

inn::Position* inn::Neuron::Synaps::getPos() const {
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

inn::WaveType inn::Neuron::Synaps::getWTs() const {
    return WTs;
}

double inn::Neuron::Synaps::getGamma() const {
    return Gamma;
}

double inn::Neuron::Synaps::getdGamma() const {
    return dGamma;
}

unsigned long long inn::Neuron::Synaps::getQSize() {
    return QSize;
}
