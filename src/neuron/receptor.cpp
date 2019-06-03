/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/receptor.cpp
// Purpose:     Neuron receptor class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"

inn::Neuron::Receptor::Receptor() {
    RPos = RPos0 = RPosf = inn::Position();
    k3 = 0;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
}

inn::Neuron::Receptor::Receptor(const Receptor &R) {
    CP = R.getCP();
    CPf = R.getCPf();
    RPos = R.getPos();
    RPos0 = R.getPos0();
    RPosf = R.getPosf();
    k3 = R.getk3();
    Rs = R.getSensitivityValue();
    Locked = R.isLocked();
    L = R.getL();
    Lf = R.getLf();
}

inn::Neuron::Receptor::Receptor(inn::Position _RPos, double _k3) {
    RPos = RPos0 = RPosf = _RPos;
    k3 = _k3;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
}

bool inn::Neuron::Receptor::doCheckActive() {
    return Fi >= Rs;
}

void inn::Neuron::Receptor::doLock() {
    Locked = true;
}

void inn::Neuron::Receptor::doUnlock() {
    Locked = false;
}

void inn::Neuron::Receptor::doReset() {
    CPf.clear();
    RPosf = RPos0;
    Rs = 0.01;
    Lf = 0;
    Fi = 0;
}

void inn::Neuron::Receptor::doSavePos() {
    if (Locked) CPf.push_back(RPosf);
    else CP.push_back(RPos);
}

void inn::Neuron::Receptor::doUpdateSensitivityValue() {
    Rs = inn::Neuron::System::getRcValue(k3, Rs, Fi, dFi);
}

void inn::Neuron::Receptor::setPos(inn::Position _RPos) {
    if (Locked) {
        Lf += inn::Position::getDistance(RPosf, _RPos);
        RPosf = RPosf + _RPos;
    } else {
        L += inn::Position::getDistance(RPos, _RPos);
        RPos = RPos + _RPos;
    }
}

void inn::Neuron::Receptor::setFi(double _Fi) {
    dFi = _Fi - Fi;
    Fi = _Fi;
}

std::vector<inn::Position> inn::Neuron::Receptor::getCP() const {
    return CP;
}

std::vector<inn::Position> inn::Neuron::Receptor::getCPf() const {
    return CPf;
}

inn::Position inn::Neuron::Receptor::getPos() const {
    return RPos;
}

inn::Position inn::Neuron::Receptor::getPos0() const {
    return RPos0;
}

inn::Position inn::Neuron::Receptor::getPosf() const {
    return RPosf;
}

double inn::Neuron::Receptor::getk3() const {
    return k3;
}

double inn::Neuron::Receptor::getSensitivityValue() const {
    return Rs;
}

bool inn::Neuron::Receptor::isLocked() const {
    return Locked;
}

double inn::Neuron::Receptor::getL() const {
    return L;
}

double inn::Neuron::Receptor::getLf() const {
    return Lf;
}


