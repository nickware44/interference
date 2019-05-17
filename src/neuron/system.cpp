/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 29.04.19
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/neuron.h"

std::vector<double> inn::Neuron::System::doCompareCPFunction(std::vector<inn::Position> CP, std::vector<inn::Position> CPf) {
    std::vector<double> R;
    unsigned long long L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    R.reserve(L);
    for (long long i = 0; i < L; i++) R.push_back(inn::Position::getDistance(CP[i], CPf[i]));
    return R;
}

double inn::Neuron::System::doCompareCPFunctionD(std::vector<inn::Position> CP, std::vector<inn::Position> CPf) {
    double R = 0;
    unsigned long long L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    for (int i = 0; i < L; i++) R += inn::Position::getDistance(CP[i], CPf[i]);
    return R;
}

double inn::Neuron::System::doCompareFunction(inn::Position R, inn::Position Rf, double, double) {
    return inn::Position::getDistance(R, Rf);
}

double inn::Neuron::System::getGammaFunctionValue(double oG, double k1, double k2, double Xt, int Type) {
    double nGamma;
    if (!Type) nGamma = oG + (k1*Xt - (1-Xt)*oG*k2);
    else nGamma = oG + (k1*(1-Xt) - Xt*oG*k2);
    return nGamma;
}

double inn::Neuron::System::getFiFunctionValue(inn::Position S, inn::Position R, double Lambda, double Gamma) {
    return Gamma * Lambda * exp(-Lambda*inn::Position::getDistance(S, R));
}

double inn::Neuron::System::getRcValue(double k3, double Rs, double Fi, double dFi) {
    if (Fi >= Rs && dFi > 0) Rs += dFi;
    else Rs = Rs / (k3*Rs+1);
    return Rs;
}

inn::Position inn::Neuron::System::getNewPosition(inn::Position R, inn::Position S, double FiL) {
    return (R-S) / inn::Position::getDistance(S, R) * FiL;
}

double inn::Neuron::System::getFiVectorLength(double dFi) {
    return sqrt(dFi);
}
