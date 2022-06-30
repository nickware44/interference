/////////////////////////////////////////////////////////////////////////////
// Name:        neuron/system.cpp
// Purpose:     Definitions of general math model functions
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuron.h"

std::vector<double> inn::Neuron::System::doCompareCPFunction(std::vector<inn::Position*> CP, std::vector<inn::Position*> CPf) {
    std::vector<double> R;
    unsigned long long L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    R.reserve(L);
    for (long long i = 0; i < L; i++) R.push_back(inn::Position::getDistance(CP[i], CPf[i]));
    return R;
}

double inn::Neuron::System::doCompareCPFunctionD(std::vector<inn::Position*> CP, std::vector<inn::Position*> CPf) {
    double R = 0;
    unsigned long long L = CP.size();
    if (CPf.size() < L) L = CPf.size();
    for (int i = 0; i < L; i++) R += inn::Position::getDistance(CP[i], CPf[i]);
    return R;
}

double inn::Neuron::System::doCompareFunction(inn::Position *R, inn::Position *Rf) {
    return 1;//inn::Position::getDistance(R, Rf);
}

double inn::Neuron::System::getGammaFunctionValue(double oG, double k1, double k2, double Xt, double WVSum) {
    double nGamma;
    if (WVSum < 13) WVSum = 1;
    nGamma = oG + (k1*Xt-(1-Xt)*oG*k2)/WVSum;
    return nGamma;
}

std::pair<double, double> inn::Neuron::System::getFiFunctionValue(double Lambda, double Gamma, double dGamma, double D) {
    double E = Lambda * exp(-Lambda*D);
    return std::make_pair(Gamma*E, dGamma*E);
}

double inn::Neuron::System::getReceptorInfluenceValue(bool Active, double dFi, inn::Position *RPos, inn::Position *RPr) {
    double Yn = 0;
    if (RPos->getDistanceFrom(RPr)) Yn = Active * dFi * (RPos->getPositionValue(1)-RPr->getPositionValue(1)) / RPos->getDistanceFrom(RPr);
    return Yn;
}

double inn::Neuron::System::getRcValue(double k3, double Rs, double Fi, double dFi) {
    if (Fi >= Rs && dFi > 0) Rs += dFi;
    else Rs = Rs / (k3*Rs+1);
    return Rs;
}

void inn::Neuron::System::getNewPosition(inn::Position *nRPos, inn::Position *R, inn::Position *S, double FiL, double D) {
    nRPos -> setPosition(R);
    nRPos -> doSubtract(S);
    nRPos -> doDivide(D);
    nRPos -> doMultiply(FiL);
}

double inn::Neuron::System::getLambdaValue(unsigned int Xm) {
    return pow(10, -(log(Xm)/log(2)-6));
}

unsigned long long inn::Neuron::System::getOutputSignalQMaxSizeValue(unsigned int Xm) {
    return pow(10, log(Xm)/log(2)-4);
}

unsigned long long inn::Neuron::System::getGammaQMaxSizeValue(double Lambda) {
    return 1/Lambda*pow(10, 2);
}

double inn::Neuron::System::getFiVectorLength(double dFi) {
    return sqrt(dFi);
}

double inn::Neuron::System::getSynapticSensitivityValue(unsigned int W, unsigned int OW) {
    double S = double(W) / OW;
    return 39.682*S*S*S*S - 170.22*S*S*S + 267.81*S*S - 178.8*S + 43.072;
}
