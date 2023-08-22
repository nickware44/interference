/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 17.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "../include/inn/computer.h"

inn::Computer::Computer() {

}

std::vector<float> inn::Computer::doCompareCPFunction(std::vector<inn::Position*> CP, std::vector<inn::Position*> CPf) {
    std::vector<float> R;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    R.reserve(L);
    for (long long i = 0; i < L; i++) R.push_back(inn::Position::getDistance(CP[i], CPf[i]));
    return R;
}

float inn::Computer::doCompareCPFunctionD(std::vector<inn::Position*> CP, std::vector<inn::Position*> CPf) {
    float R = 0;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    for (int i = 0; i < L; i++) R += inn::Position::getDistance(CP[i], CPf[i]);
    return R;
}

float inn::Computer::doCompareFunction(inn::Position *R, inn::Position *Rf) {
    return inn::Position::getDistance(R, Rf);
}

float inn::Computer::getGammaFunctionValue(float oG, float k1, float k2, float Xt) {
    float nGamma;
    nGamma = oG + (k1*Xt-oG/k2);
    return nGamma;
}

std::pair<float, float> inn::Computer::getFiFunctionValue(float Lambda, float Gamma, float dGamma, float D) {
    float E = Lambda * std::exp(-Lambda*D);
    return std::make_pair(Gamma*E, dGamma*E);
}

float inn::Computer::getReceptorInfluenceValue(bool Active, float dFi, inn::Position *dPos, inn::Position *RPr) {
    float Yn = 0;
    auto d = dPos -> getDistanceFrom(RPr);
    if (d > 0 && Active) Yn = d; // TODO: research output variants
    return Yn;
}

float inn::Computer::getRcValue(float k3, float Rs, float Fi, float dFi) {
    if (Fi >= Rs && dFi > 0) Rs += dFi;
    else Rs = Rs / (k3*Rs+1);
    return Rs;
}

void inn::Computer::getNewPosition(inn::Position *nRPos, inn::Position *R, inn::Position *S, float FiL, float D) {
    nRPos -> setPosition(R);
    nRPos -> doSubtract(S);
    nRPos -> doDivide(D);
    nRPos -> doMultiply(FiL);
}

float inn::Computer::getLambdaValue(unsigned int Xm) {
    return pow(10, -(log(Xm)/log(2)-6));
}

float inn::Computer::getFiVectorLength(float dFi) {
    return sqrt(dFi);
}

float inn::Computer::getSynapticSensitivityValue(unsigned int W, unsigned int OW) {
    float S = float(W) / OW;
    return 39.682*S*S*S*S - 170.22*S*S*S + 267.81*S*S - 178.8*S + 43.072;
}
