/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/backends/default.h"

std::vector<double> inn::ComputeBackendDefault::doCompareCPFunction(std::vector<inn::Position*> CP, std::vector<inn::Position*> CPf) {
    std::vector<double> R;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    R.reserve(L);
    for (long long i = 0; i < L; i++) R.push_back(inn::Position::getDistance(CP[i], CPf[i]));
    return R;
}

double inn::ComputeBackendDefault::doCompareCPFunctionD(std::vector<inn::Position*> CP, std::vector<inn::Position*> CPf) {
    double R = 0;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    for (int i = 0; i < L; i++) R += inn::Position::getDistance(CP[i], CPf[i]);
    return R;
}

double inn::ComputeBackendDefault::doCompareFunction(inn::Position *R, inn::Position *Rf) {
    return 1;//inn::Position::getDistance(R, Rf);
}

double inn::ComputeBackendDefault::getGammaFunctionValue(double oG, double k1, double k2, double Xt, double WVSum) {
    double nGamma;
    if (WVSum < 13) WVSum = 1;
    nGamma = oG + (k1*Xt-(1-Xt)*oG*k2)/WVSum;
    return nGamma;
}

std::pair<double, double> inn::ComputeBackendDefault::getFiFunctionValue(double Lambda, double Gamma, double dGamma, double D) {
    double E = Lambda * exp(-Lambda*D);
    return std::make_pair(Gamma*E, dGamma*E);
}

double inn::ComputeBackendDefault::getReceptorInfluenceValue(bool Active, double dFi, inn::Position *RPos, inn::Position *RPr) {
    double Yn = 0;
    if (RPos->getDistanceFrom(RPr)) Yn = Active * dFi * (RPos->getPositionValue(1)-RPr->getPositionValue(1)) / RPos->getDistanceFrom(RPr);
    return Yn;
}

double inn::ComputeBackendDefault::getRcValue(double k3, double Rs, double Fi, double dFi) {
    if (Fi >= Rs && dFi > 0) Rs += dFi;
    else Rs = Rs / (k3*Rs+1);
    return Rs;
}

void inn::ComputeBackendDefault::getNewPosition(inn::Position *nRPos, inn::Position *R, inn::Position *S, double FiL, double D) {
    nRPos -> setPosition(R);
    nRPos -> doSubtract(S);
    nRPos -> doDivide(D);
    nRPos -> doMultiply(FiL);
}

double inn::ComputeBackendDefault::getLambdaValue(unsigned int Xm) {
    return pow(10, -(log(Xm)/log(2)-6));
}

int64_t inn::ComputeBackendDefault::getOutputSignalQMaxSizeValue(unsigned int Xm) {
    return pow(10, log(Xm)/log(2)-4);
}

int64_t inn::ComputeBackendDefault::getGammaQMaxSizeValue(double Lambda) {
    return 1/Lambda*pow(10, 2);
}

double inn::ComputeBackendDefault::getFiVectorLength(double dFi) {
    return sqrt(dFi);
}

double inn::ComputeBackendDefault::getSynapticSensitivityValue(unsigned int W, unsigned int OW) {
    double S = double(W) / OW;
    return 39.682*S*S*S*S - 170.22*S*S*S + 267.81*S*S - 178.8*S + 43.072;
}
