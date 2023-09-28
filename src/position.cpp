/////////////////////////////////////////////////////////////////////////////
// Name:        position.cpp
// Purpose:     Multidimensional position class
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include <indk/position.h>
#include <indk/error.h>

indk::Position::Position() {
    Xm = 0;
    DimensionsCount = 0;
    X = nullptr;
}

indk::Position::Position(const indk::Position &P) {
    Xm = P.getXm();
    DimensionsCount = P.getDimensionsCount();
    X = new float[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
}

indk::Position::Position(unsigned int _Xm, unsigned int _DimensionsCount) {
    Xm = _Xm;
    DimensionsCount = _DimensionsCount;
    X = new float[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = 0;
}

indk::Position::Position(unsigned int _Xm, std::vector<float> _X) {
    Xm = _Xm;
    DimensionsCount = (unsigned)_X.size();
    X = new float[DimensionsCount];
    for (int i = 0; i < DimensionsCount; i++) {
        if (_X[i] < 0 || _X[i] > Xm) {
            throw indk::Error(indk::Error::EX_POSITION_OUT_RANGES, {_X[i], (float)Xm});
        }
        X[i] = _X[i];
    }
}

void indk::Position::doAdd(const indk::Position *P) {
    if (DimensionsCount != P->getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    if (Xm != P->getXm()) {
        throw indk::Error(indk::Error::EX_POSITION_RANGES);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] += P->getPositionValue(i);
        if (X[i] > Xm) {
            throw indk::Error(indk::Error::EX_POSITION_OUT_RANGES, {X[i], (float)Xm});
        }
    }
}

void indk::Position::doSubtract(const indk::Position *P) {
    if (DimensionsCount != P->getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    if (Xm != P->getXm()) {
        throw indk::Error(indk::Error::EX_POSITION_RANGES);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] = (X[i]-P->getPositionValue(i));
        if (X[i] > Xm) {
            throw indk::Error(indk::Error::EX_POSITION_OUT_RANGES, {X[i], (float)Xm});
        }
    }
}

void indk::Position::doDivide(float D) {
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] /= D;
    }
}

void indk::Position::doMultiply(float M) {
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] *= M;
        if (X[i] > Xm) {
            throw indk::Error(indk::Error::EX_POSITION_OUT_RANGES, {X[i], (float)Xm});
        }
    }
}

void indk::Position::doZeroPosition() {
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = 0;
}

void indk::Position::setPosition(const indk::Position &P) {
    if (P.getDimensionsCount() < DimensionsCount) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
}

void indk::Position::setPosition(const indk::Position *P) {
    if (P->getDimensionsCount() < DimensionsCount) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P -> getPositionValue(i);
}

void indk::Position::setPosition(std::vector<float> _X) {
    if (_X.size() != DimensionsCount) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    for (int i = 0; i < DimensionsCount; i++) {
        if (_X[i] < 0 || _X[i] > Xm) {
            throw indk::Error(indk::Error::EX_POSITION_OUT_RANGES, {_X[i], (float)Xm});
        }
        X[i] = _X[i];
    }
}

void indk::Position::setXm(unsigned int _Xm) {
    Xm = _Xm;
}

void indk::Position::setDimensionsCount(unsigned int _DimensionsCount) {
    DimensionsCount = _DimensionsCount;
}

unsigned int indk::Position::getDimensionsCount() const {
    return DimensionsCount;
}

unsigned int indk::Position::getXm() const {
    return Xm;
}

float indk::Position::getPositionValue(unsigned int DNum) const {
    if (DNum >= DimensionsCount) return -1;
    return X[DNum];
}

float indk::Position::getDistanceFrom(const indk::Position *P) {
    if (DimensionsCount != P->getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    float D = 0, CValue;
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        CValue = X[i] - P -> getPositionValue(i);
        D += CValue * CValue;
    }
    return sqrt(D);
}

indk::Position::~Position() {
    delete [] X;
}

indk::Position& indk::Position::operator=(const indk::Position &P) {
	if (this == &P) return *this;
	delete X;
    Xm = P.getXm();
    DimensionsCount = P.getDimensionsCount();
	X = new float[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
    return *this;
}

indk::Position indk::operator+(const indk::Position &L, const indk::Position &R) {
	if (L.getDimensionsCount() != R.getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    if (L.getXm() != R.getXm()) {
        throw indk::Error(indk::Error::EX_POSITION_RANGES);
    }
    std::vector<float> PV;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        PV.push_back(L.getPositionValue(i)+R.getPositionValue(i));
    }
    return indk::Position(L.getXm(), PV);
}

indk::Position indk::operator-(const indk::Position &L, const indk::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    if (L.getXm() != R.getXm()) {
        throw indk::Error(indk::Error::EX_POSITION_RANGES);
    }
    std::vector<float> PV;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        PV.push_back(fabs(L.getPositionValue(i)-R.getPositionValue(i)));
    }
    return indk::Position(L.getXm(), PV);
}

indk::Position indk::operator/(const indk::Position &P, float D) {
    std::vector<float> PV;
    for (unsigned int i = 0; i < P.getDimensionsCount(); i++) {
        PV.push_back(P.getPositionValue(i)/D);
    }
    return indk::Position(P.getXm(), PV);
}

indk::Position indk::operator*(const indk::Position &P, float M) {
    std::vector<float> PV;
    for (unsigned int i = 0; i < P.getDimensionsCount(); i++) {
        PV.push_back(P.getPositionValue(i)*M);
    }
    return indk::Position(P.getXm(), PV);
}

float indk::Position::getDistance(const indk::Position &L, const indk::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    float D = 0;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        D += (L.getPositionValue(i)-R.getPositionValue(i))*(L.getPositionValue(i)-R.getPositionValue(i));
    }
    return std::sqrt(D);
}

indk::Position* indk::Position::getSum(const indk::Position *L, const indk::Position *R) {
    if (L->getDimensionsCount() != R->getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    if (L->getXm() != R->getXm()) {
        throw indk::Error(indk::Error::EX_POSITION_RANGES);
    }
    std::vector<float> PV;
    for (unsigned int i = 0; i < L->getDimensionsCount(); i++) {
        PV.push_back(L->getPositionValue(i)+R->getPositionValue(i));
    }
    return new indk::Position(L->getXm(), PV);
}

indk::Position* indk::Position::getDiff(const indk::Position *L, const indk::Position *R) {
    if (L->getDimensionsCount() != R->getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    if (L->getXm() != R->getXm()) {
        throw indk::Error(indk::Error::EX_POSITION_RANGES);
    }
    std::vector<float> PV;
    for (unsigned int i = 0; i < L->getDimensionsCount(); i++) {
        PV.push_back(fabs(L->getPositionValue(i)-R->getPositionValue(i)));
    }
    return new indk::Position(L->getXm(), PV);
}

indk::Position* indk::Position::getQuotient(const indk::Position *P, float D) {
    std::vector<float> PV;
    for (unsigned int i = 0; i < P->getDimensionsCount(); i++) {
        PV.push_back(P->getPositionValue(i)/D);
    }
    return new indk::Position(P->getXm(), PV);
}

indk::Position* indk::Position::getProduct(const indk::Position *P, float M) {
    std::vector<float> PV;
    for (unsigned int i = 0; i < P->getDimensionsCount(); i++) {
        PV.push_back(P->getPositionValue(i)*M);
    }
    return new indk::Position(P->getXm(), PV);
}

float indk::Position::getDistance(const indk::Position *L, const indk::Position *R) {
    if (L->getDimensionsCount() != R->getDimensionsCount()) {
        throw indk::Error(indk::Error::EX_POSITION_DIMENSIONS);
    }
    float D = 0;
    for (unsigned int i = 0; i < L->getDimensionsCount(); i++) {
        D += (L->getPositionValue(i)-R->getPositionValue(i))*(L->getPositionValue(i)-R->getPositionValue(i));
    }
    return std::sqrt(D);
}
