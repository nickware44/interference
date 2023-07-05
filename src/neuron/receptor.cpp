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
	DefaultPos = new inn::Position();
	PhantomPos = new inn::Position();
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
	DefaultPos = new inn::Position(*R.getPos0());
	PhantomPos = new inn::Position(*R.getPosf());
    k3 = R.getk3();
    Rs = R.getSensitivityValue();
    Locked = R.isLocked();
    L = R.getL();
    Lf = R.getLf();
    Fi = 0;
    dFi = 0;
}

inn::Neuron::Receptor::Receptor(inn::Position *_RPos, float _k3) {
	DefaultPos = new inn::Position(*_RPos);
	PhantomPos = new inn::Position(*_RPos);
    k3 = _k3;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
    dFi = 0;
}

bool inn::Neuron::Receptor::doCheckActive() const {
    return Fi >= Rs;
}

void inn::Neuron::Receptor::doLock() {
    Locked = true;
}

void inn::Neuron::Receptor::doUnlock() {
    Locked = false;
}

void inn::Neuron::Receptor::doCreateNewScope() {
    auto pos = new inn::Position(DefaultPos->getXm(), DefaultPos->getDimensionsCount());
    Scope = ReferencePos.size();
    ReferencePos.push_back(pos);
}

void inn::Neuron::Receptor::doChangeScope(uint64_t scope) {
    if (scope > ReferencePos.size()) {
        Scope = ReferencePos.size() - 1;
        return;
    }
    Scope = scope;
}

void inn::Neuron::Receptor::doReset() {
    CPf.clear();
    Rs = 0.01;
    Lf = 0;
    Fi = 0;
    dFi = 0;
    ReferencePos.clear();
    PhantomPos -> setPosition(DefaultPos);
    Locked = false;
}

void inn::Neuron::Receptor::doPrepare() {
    CPf.clear();
    Rs = 0.01;
    Lf = 0;
    Fi = 0;
    dFi = 0;
    if (Locked) PhantomPos -> setPosition(DefaultPos);
    else ReferencePos[Scope] -> setPosition(DefaultPos);
}

void inn::Neuron::Receptor::doSavePos() {
    if (Locked) CPf.push_back(new inn::Position(*PhantomPos));
    else CP.push_back(new inn::Position(*ReferencePos[Scope]));
}

void inn::Neuron::Receptor::doUpdateSensitivityValue() {
    Rs = inn::Computer::getRcValue(k3, Rs, Fi, dFi);
}

void inn::Neuron::Receptor::setPos(inn::Position *_RPos) {
    if (Locked) {
        Lf += inn::Position::getDistance(PhantomPos, _RPos);
        PhantomPos -> doAdd(_RPos);
    } else {
        L += inn::Position::getDistance(ReferencePos[Scope], _RPos);
        ReferencePos[Scope] -> doAdd(_RPos);
    }
}

void inn::Neuron::Receptor::setRs(float _Rs) {
    Rs = _Rs;
}

void inn::Neuron::Receptor::setk3(float _k3) {
    k3 = _k3;
}

void inn::Neuron::Receptor::setFi(float _Fi) {
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
    return ReferencePos[Scope];
}

inn::Position* inn::Neuron::Receptor::getPos0() const {
    return DefaultPos;
}

inn::Position* inn::Neuron::Receptor::getPosf() const {
    return PhantomPos;
}

std::vector<inn::Position*> inn::Neuron::Receptor::getReferencePosScopes() {
    return ReferencePos;
}

float inn::Neuron::Receptor::getRs() const {
    return Rs;
}

float inn::Neuron::Receptor::getk3() const {
    return k3;
}

float inn::Neuron::Receptor::getFi() {
    return Fi;
}

float inn::Neuron::Receptor::getdFi() {
    return dFi;
}

float inn::Neuron::Receptor::getSensitivityValue() const {
    return Rs;
}

bool inn::Neuron::Receptor::isLocked() const {
    return Locked;
}

float inn::Neuron::Receptor::getL() const {
    return L;
}

float inn::Neuron::Receptor::getLf() const {
    return Lf;
}
