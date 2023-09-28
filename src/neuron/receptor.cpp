/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/receptor.cpp
// Purpose:     Neuron receptor class
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/neuron.h>
#include <indk/system.h>

indk::Neuron::Receptor::Receptor() {
	DefaultPos = new indk::Position();
	PhantomPos = new indk::Position();
    k3 = 0;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
    dFi = 0;
}

indk::Neuron::Receptor::Receptor(const Receptor &R) {
    CP = R.getCP();
    CPf = R.getCPf();
	DefaultPos = new indk::Position(*R.getPos0());
	PhantomPos = new indk::Position(*R.getPosf());
    k3 = R.getk3();
    Rs = R.getSensitivityValue();
    Locked = R.isLocked();
    L = R.getL();
    Lf = R.getLf();
    Fi = 0;
    dFi = 0;
}

indk::Neuron::Receptor::Receptor(indk::Position *_RPos, float _k3) {
	DefaultPos = new indk::Position(*_RPos);
	PhantomPos = new indk::Position(*_RPos);
    k3 = _k3;
    Rs = 0.01;
    Locked = false;
    L = 0;
    Lf = 0;
    Fi = 0;
    dFi = 0;
}

bool indk::Neuron::Receptor::doCheckActive() const {
    return Fi >= Rs;
}

void indk::Neuron::Receptor::doLock() {
    Locked = true;
}

void indk::Neuron::Receptor::doUnlock() {
    Locked = false;
}

void indk::Neuron::Receptor::doCreateNewScope() {
    auto pos = new indk::Position(DefaultPos->getXm(), DefaultPos->getDimensionsCount());
    Scope = ReferencePos.size();
    ReferencePos.push_back(pos);
}

void indk::Neuron::Receptor::doChangeScope(uint64_t scope) {
    if (scope > ReferencePos.size()) {
        Scope = ReferencePos.size() - 1;
        return;
    }
    Scope = scope;
}

void indk::Neuron::Receptor::doReset() {
    CPf.clear();
    Rs = 0.01;
    Lf = 0;
    Fi = 0;
    dFi = 0;
    ReferencePos.clear();
    PhantomPos -> setPosition(DefaultPos);
    Locked = false;
}

void indk::Neuron::Receptor::doPrepare() {
    CPf.clear();
    Rs = 0.01;
    Lf = 0;
    Fi = 0;
    dFi = 0;
    if (Locked) PhantomPos -> setPosition(DefaultPos);
    else ReferencePos[Scope] -> setPosition(DefaultPos);
}

void indk::Neuron::Receptor::doSavePos() {
    if (Locked) CPf.push_back(new indk::Position(*PhantomPos));
    else CP.push_back(new indk::Position(*ReferencePos[Scope]));
}

void indk::Neuron::Receptor::doUpdateSensitivityValue() {
    Rs = indk::Computer::getRcValue(k3, Rs, Fi, dFi);
}

void indk::Neuron::Receptor::doUpdatePos(indk::Position *_RPos) {
    if (Locked) {
        Lf += indk::Position::getDistance(PhantomPos, _RPos);
        PhantomPos -> doAdd(_RPos);
    } else {
        L += indk::Position::getDistance(ReferencePos[Scope], _RPos);
        ReferencePos[Scope] -> doAdd(_RPos);
    }
}

void indk::Neuron::Receptor::setPos(indk::Position *_RPos) {
    if (Locked) {
        PhantomPos -> setPosition(_RPos);
    } else {
        ReferencePos[Scope] -> setPosition(_RPos);
    }
}

void indk::Neuron::Receptor::setRs(float _Rs) {
    Rs = _Rs;
}

void indk::Neuron::Receptor::setk3(float _k3) {
    k3 = _k3;
}

void indk::Neuron::Receptor::setFi(float _Fi) {
    dFi = _Fi - Fi;
    Fi = _Fi;
}

std::vector<indk::Position*> indk::Neuron::Receptor::getCP() const {
    return CP;
}

std::vector<indk::Position*> indk::Neuron::Receptor::getCPf() const {
    return CPf;
}

indk::Position* indk::Neuron::Receptor::getPos() const {
    return ReferencePos[Scope];
}

indk::Position* indk::Neuron::Receptor::getPos0() const {
    return DefaultPos;
}

indk::Position* indk::Neuron::Receptor::getPosf() const {
    return PhantomPos;
}

std::vector<indk::Position*> indk::Neuron::Receptor::getReferencePosScopes() {
    return ReferencePos;
}

float indk::Neuron::Receptor::getRs() const {
    return Rs;
}

float indk::Neuron::Receptor::getk3() const {
    return k3;
}

float indk::Neuron::Receptor::getFi() {
    return Fi;
}

float indk::Neuron::Receptor::getdFi() {
    return dFi;
}

float indk::Neuron::Receptor::getSensitivityValue() const {
    return Rs;
}

bool indk::Neuron::Receptor::isLocked() const {
    return Locked;
}

float indk::Neuron::Receptor::getL() const {
    return L;
}

float indk::Neuron::Receptor::getLf() const {
    return Lf;
}
