/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/receptor.cpp
// Purpose:     Neuron receptor class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"
#include "../../include/inn/system.h"

inn::Neuron::Receptor::Receptor() {
	RPos = new inn::Position();
	RPos0 = new inn::Position();
	RPosf = new inn::Position();
    k3 = 0;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
    dFi = 0;
}

inn::Neuron::Receptor::Receptor(const Receptor &R) {
    CP = R.getCP();
    CPf = R.getCPf();
	RPos = new inn::Position(*R.getPos());
	RPos0 = new inn::Position(*R.getPos0());
	RPosf = new inn::Position(*R.getPosf());
    k3 = R.getk3();
    Rs = R.getSensitivityValue();
    Locked = R.isLocked();
    L = R.getL();
    Lf = R.getLf();
    Fi = 0;
    dFi = 0;
}

inn::Neuron::Receptor::Receptor(inn::Position *_RPos, double _k3) {
	RPos = _RPos;
	RPos0 = new inn::Position(*_RPos);
	RPosf = new inn::Position(*_RPos);
    k3 = _k3;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
    dFi = 0;
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
    Rs = 0.01;
    Lf = 0;
    Fi = 0;
    dFi = 0;
}

void inn::Neuron::Receptor::doPrepare() {
    RPosf -> setPosition(RPos0);
}

void inn::Neuron::Receptor::doSavePos() {
    if (Locked) CPf.push_back(new inn::Position(*RPosf));
    else CP.push_back(new inn::Position(*RPos));
}

void inn::Neuron::Receptor::doUpdateSensitivityValue() {
    Rs = inn::Computer::getRcValue(k3, Rs, Fi, dFi);
}

void inn::Neuron::Receptor::setPos(inn::Position *_RPos) {
    if (Locked) {
        Lf += inn::Position::getDistance(RPosf, _RPos);
        RPosf -> doAdd(_RPos);
    } else {
        L += inn::Position::getDistance(RPos, _RPos);
        RPos -> doAdd(_RPos);
    }
}

void inn::Neuron::Receptor::setk3(double _k3) {
    k3 = _k3;
}

void inn::Neuron::Receptor::setFi(double _Fi) {
    dFi = _Fi - Fi;
    Fi = _Fi;
}

std::vector<inn::Position*> inn::Neuron::Receptor::getCP() const {
    return CP;
}

std::vector<inn::Position*> inn::Neuron::Receptor::getCPf() const {
    return CPf;
}

inn::Position* inn::Neuron::Receptor::getPos() const {
    return RPos;
}

inn::Position* inn::Neuron::Receptor::getPos0() const {
    return RPos0;
}

inn::Position* inn::Neuron::Receptor::getPosf() const {
    return RPosf;
}

double inn::Neuron::Receptor::getk3() const {
    return k3;
}

double inn::Neuron::Receptor::getdFi() {
    return dFi;
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


