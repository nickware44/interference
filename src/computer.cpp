/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 17.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <indk/computer.h>

indk::Computer::Computer() {

}

std::vector<float> indk::Computer::doCompareCPFunction(std::vector<indk::Position*> CP, std::vector<indk::Position*> CPf) {
    std::vector<float> R;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    R.reserve(L);
    for (long long i = 0; i < L; i++) R.push_back(indk::Position::getDistance(CP[i], CPf[i]));
    return R;
}

float indk::Computer::doCompareCPFunctionD(std::vector<indk::Position*> CP, std::vector<indk::Position*> CPf) {
    float R = 0;
    int64_t L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    for (int i = 0; i < L; i++) R += indk::Position::getDistance(CP[i], CPf[i]);
    return R;
}

float indk::Computer::doCompareFunction(indk::Position *R, indk::Position *Rf) {
    return indk::Position::getDistance(R, Rf);
}

float indk::Computer::getGammaFunctionValue(float oG, float k1, float k2, float Xt) {
    float nGamma;
    nGamma = oG + (k1*Xt-oG/k2);
    return nGamma;
}

std::pair<float, float> indk::Computer::getFiFunctionValue(float Lambda, float Gamma, float dGamma, float D) {
    float E = Lambda * std::exp(-Lambda*D);
    return std::make_pair(Gamma*E, dGamma*E);
}

float indk::Computer::getReceptorInfluenceValue(bool Active, float dFi, indk::Position *dPos, indk::Position *RPr) {
    float Yn = 0;
    auto d = dPos -> getDistanceFrom(RPr);
    if (d > 0 && Active) Yn = d; // TODO: research output variants
    return Yn;
}

float indk::Computer::getRcValue(float k3, float Rs, float Fi, float dFi) {
    if (Fi >= Rs && dFi > 0) Rs += dFi / k3;
    else Rs = Rs / (k3*Rs+1);
    return Rs;
}

void indk::Computer::getNewPosition(indk::Position *nRPos, indk::Position *R, indk::Position *S, float FiL, float D) {
    nRPos -> setPosition(R);
    nRPos -> doSubtract(S);
    nRPos -> doDivide(D);
    nRPos -> doMultiply(FiL);
}

float indk::Computer::getLambdaValue(unsigned int Xm) {
    return pow(10, -(log(Xm)/log(2)-6));
}

float indk::Computer::getFiVectorLength(float dFi) {
    return sqrt(dFi);
}

float indk::Computer::getSynapticSensitivityValue(unsigned int W, unsigned int OW) {
    float S = float(W) / OW;
    return 39.682*S*S*S*S - 170.22*S*S*S + 267.81*S*S - 178.8*S + 43.072;
}
