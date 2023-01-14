/////////////////////////////////////////////////////////////////////////////
// Name:        position.cpp
// Purpose:     Multidimensional position class
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../include/inn/position.h"
#include "../include/inn/error.h"

inn::Position::Position() {
    Xm = 0;
    DimensionsCount = 0;
    X = nullptr;
}

inn::Position::Position(const inn::Position &P) {
    Xm = P.getXm();
    DimensionsCount = P.getDimensionsCount();
    X = new double[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
}

inn::Position::Position(unsigned int _Xm, unsigned int _DimensionsCount) {
    Xm = _Xm;
    DimensionsCount = _DimensionsCount;
    X = new double[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = 0;
}

inn::Position::Position(unsigned int _Xm, std::vector<double> _X) {
    Xm = _Xm;
    DimensionsCount = (unsigned)_X.size();
    X = new double[DimensionsCount];
    for (int i = 0; i < DimensionsCount; i++) {
        if (_X[i] < 0 || _X[i] > Xm) {
//            throw inn::Error(inn::Error::EX_POSITION_OUT_RANGES, {_X[i], (double)Xm});
        }
        X[i] = _X[i];
    }
}

void inn::Position::doAdd(const inn::Position *P) {
    if (DimensionsCount != P->getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    if (Xm != P->getXm()) {
        throw inn::Error(inn::Error::EX_POSITION_RANGES);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] += P->getPositionValue(i);
        if (X[i] < 0 || X[i] > Xm) {
//            throw inn::Error(inn::Error::EX_POSITION_OUT_RANGES, {X[i], (double)Xm});
        }
    }
}

void inn::Position::doSubtract(const inn::Position *P) {
    if (DimensionsCount != P->getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    if (Xm != P->getXm()) {
        throw inn::Error(inn::Error::EX_POSITION_RANGES);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] = fabs(X[i]-P->getPositionValue(i));
        if (X[i] < 0 || X[i] > Xm) {
//            throw inn::Error(inn::Error::EX_POSITION_OUT_RANGES, {X[i], (double)Xm});
        }
    }
}

void inn::Position::doDivide(double D) {
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] /= D;
    }
}

void inn::Position::doMultiply(double M) {
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] *= M;
        if (X[i] < 0 || X[i] > Xm) {
//            throw inn::Error(inn::Error::EX_POSITION_OUT_RANGES, {X[i], (double)Xm});
        }
    }
}

void inn::Position::doZeroPosition() {
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = 0;
}

void inn::Position::setPosition(const inn::Position &P) {
    if (P.getDimensionsCount() < DimensionsCount) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
}

void inn::Position::setPosition(const inn::Position *P) {
    if (P->getDimensionsCount() < DimensionsCount) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P -> getPositionValue(i);
}

void inn::Position::setPosition(std::vector<double> _X) {
    if (_X.size() != DimensionsCount) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    for (int i = 0; i < DimensionsCount; i++) {
        if (_X[i] < 0 || _X[i] > Xm) {
//            throw inn::Error(inn::Error::EX_POSITION_OUT_RANGES, {_X[i], (double)Xm});
        }
        X[i] = _X[i];
    }
}

unsigned int inn::Position::getDimensionsCount() const {
    return DimensionsCount;
}

unsigned int inn::Position::getXm() const {
    return Xm;
}

double inn::Position::getPositionValue(unsigned int DNum) const {
    if (DNum >= DimensionsCount) return -1;
    return X[DNum];
}

double inn::Position::getDistanceFrom(const inn::Position *P) {
    if (DimensionsCount != P->getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    double D = 0, CValue;
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        CValue = X[i] - P -> getPositionValue(i);
        D += CValue * CValue;
    }
    return sqrt(D);
}

inn::Position::~Position() {
    delete [] X;
}

inn::Position& inn::Position::operator=(const inn::Position &P) {
	if (this == &P) return *this;
	delete X;
    Xm = P.getXm();
    DimensionsCount = P.getDimensionsCount();
	X = new double[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
    return *this;
}

const inn::Position inn::operator+(const inn::Position &L, const inn::Position &R) {
	if (L.getDimensionsCount() != R.getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    if (L.getXm() != R.getXm()) {
        throw inn::Error(inn::Error::EX_POSITION_RANGES);
    }
    std::vector<double> PV;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        PV.push_back(L.getPositionValue(i)+R.getPositionValue(i));
    }
    return inn::Position(L.getXm(), PV);
}

const inn::Position inn::operator-(const inn::Position &L, const inn::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    if (L.getXm() != R.getXm()) {
        throw inn::Error(inn::Error::EX_POSITION_RANGES);
    }
    std::vector<double> PV;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        PV.push_back(fabs(L.getPositionValue(i)-R.getPositionValue(i)));
    }
    return inn::Position(L.getXm(), PV);
}

const inn::Position inn::operator/(const inn::Position &P, double D) {
    std::vector<double> PV;
    for (unsigned int i = 0; i < P.getDimensionsCount(); i++) {
        PV.push_back(P.getPositionValue(i)/D);
    }
    return inn::Position(P.getXm(), PV);
}

const inn::Position inn::operator*(const inn::Position &P, double M) {
    std::vector<double> PV;
    for (unsigned int i = 0; i < P.getDimensionsCount(); i++) {
        PV.push_back(P.getPositionValue(i)*M);
    }
    return inn::Position(P.getXm(), PV);
}

const double inn::Position::getDistance(const inn::Position &L, const inn::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    double D = 0;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        D += (L.getPositionValue(i)-R.getPositionValue(i))*(L.getPositionValue(i)-R.getPositionValue(i));
    }
    return sqrt(D);
}

inn::Position* inn::Position::getSum(const inn::Position *L, const inn::Position *R) {
    if (L->getDimensionsCount() != R->getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    if (L->getXm() != R->getXm()) {
        throw inn::Error(inn::Error::EX_POSITION_RANGES);
    }
    std::vector<double> PV;
    for (unsigned int i = 0; i < L->getDimensionsCount(); i++) {
        PV.push_back(L->getPositionValue(i)+R->getPositionValue(i));
    }
    return new inn::Position(L->getXm(), PV);
}

inn::Position* inn::Position::getDiff(const inn::Position *L, const inn::Position *R) {
    if (L->getDimensionsCount() != R->getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    if (L->getXm() != R->getXm()) {
        throw inn::Error(inn::Error::EX_POSITION_RANGES);
    }
    std::vector<double> PV;
    for (unsigned int i = 0; i < L->getDimensionsCount(); i++) {
        PV.push_back(fabs(L->getPositionValue(i)-R->getPositionValue(i)));
    }
    return new inn::Position(L->getXm(), PV);
}

inn::Position* inn::Position::getQuotient(const inn::Position *P, double D) {
    std::vector<double> PV;
    for (unsigned int i = 0; i < P->getDimensionsCount(); i++) {
        PV.push_back(P->getPositionValue(i)/D);
    }
    return new inn::Position(P->getXm(), PV);
}

inn::Position* inn::Position::getProduct(const inn::Position *P, double M) {
    std::vector<double> PV;
    for (unsigned int i = 0; i < P->getDimensionsCount(); i++) {
        PV.push_back(P->getPositionValue(i)*M);
    }
    return new inn::Position(P->getXm(), PV);
}

const double inn::Position::getDistance(const inn::Position *L, const inn::Position *R) {
    if (L->getDimensionsCount() != R->getDimensionsCount()) {
        throw inn::Error(inn::Error::EX_POSITION_DIMENSIONS);
    }
    double D = 0;
    for (unsigned int i = 0; i < L->getDimensionsCount(); i++) {
        D += (L->getPositionValue(i)-R->getPositionValue(i))*(L->getPositionValue(i)-R->getPositionValue(i));
    }
    return sqrt(D);
}
