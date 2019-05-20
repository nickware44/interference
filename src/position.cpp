/////////////////////////////////////////////////////////////////////////////
// Name:        position.cpp
// Purpose:     Multidimensional position class
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../include/inn/position.h"

inn::Position::Position() {
    DimensionsCount = 0;
    Xm = 0;
    X = nullptr;
}

inn::Position::Position(const inn::Position &P) {
    DimensionsCount = P.getDimensionsCount();
    X = new double[DimensionsCount];
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
}

inn::Position::Position(unsigned int _Xm, std::vector<double> _X) {
    Xm = _Xm;
    DimensionsCount = (unsigned)_X.size();
    X = new double[DimensionsCount];
    for (int i = 0; i < DimensionsCount; i++) {
        if (_X[i] < 0 || _X[i] > Xm) {
            // inn::Error
            return;
        }
        X[i] = _X[i];
    }
}

void inn::Position::setPosition(const inn::Position &P) {
    if (P.getDimensionsCount() < DimensionsCount) {
        // inn::Error
        return;
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) X[i] = P.getPositionValue(i);
}

void inn::Position::setPosition(std::vector<double> _X) {
    if (_X.size() != DimensionsCount) {
        // inn::Error
        return;
    }
    for (int i = 0; i < DimensionsCount; i++) {
        if (_X[i] < 0 || _X[i] > Xm) {
            // inn::Error
            return;
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

inn::Position::~Position() {
    delete [] X;
}

inn::Position& inn::Position::operator=(const inn::Position &P) {
    if (this == &P) return *this;
    if (DimensionsCount != P.getDimensionsCount() || Xm != P.getXm()) {
        // inn::Error
    }
    for (unsigned int i = 0; i < DimensionsCount; i++) {
        X[i] = P.getPositionValue(i);
        if (X[i] < 0 || X[i] > Xm) {
            // inn::Error
            return *this;
        }
    }
    return *this;
}

const inn::Position operator+(const inn::Position &L, const inn::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount() || L.getXm() != R.getXm()) {
        // inn::Error
        return inn::Position();
    }
    std::vector<double> PV;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        PV.push_back(L.getPositionValue(i)+R.getPositionValue(i));
    }
    return inn::Position(L.getXm(), PV);
}

const inn::Position operator-(const inn::Position &L, const inn::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount() || L.getXm() != R.getXm()) {
        // inn::Error
        return inn::Position();
    }
    std::vector<double> PV;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        PV.push_back(L.getPositionValue(i)-R.getPositionValue(i));
    }
    return inn::Position(L.getXm(), PV);
}

const inn::Position operator/(const inn::Position &P, double V) {
    std::vector<double> PV;
    for (unsigned int i = 0; i < P.getDimensionsCount(); i++) {
        PV.push_back(P.getPositionValue(i)/V);
    }
    return inn::Position(P.getXm(), PV);
}

const inn::Position operator*(const inn::Position &P, double V) {
    std::vector<double> PV;
    for (unsigned int i = 0; i < P.getDimensionsCount(); i++) {
        PV.push_back(P.getPositionValue(i)*V);
    }
    return inn::Position(P.getXm(), PV);
}

const double inn::Position::getDistance(const inn::Position &L, const inn::Position &R) {
    if (L.getDimensionsCount() != R.getDimensionsCount()) {
        // inn::Error
        return -1;
    }
    double D = 0;
    for (unsigned int i = 0; i < L.getDimensionsCount(); i++) {
        D += (L.getPositionValue(i)-R.getPositionValue(i))*(L.getPositionValue(i)-R.getPositionValue(i));
    }
    return sqrt(D);
}
