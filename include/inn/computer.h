/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 17.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_COMPUTER_H
#define INTERFERENCE_COMPUTER_H

#include <queue>
#include "position.h"

namespace inn {
    class Computer {
    public:
        Computer();
        virtual void doProcessNeuron(void*) = 0;
        static std::vector<double> doCompareCPFunction(std::vector<inn::Position*>, std::vector<inn::Position*>);
        static double doCompareCPFunctionD(std::vector<inn::Position*>, std::vector<inn::Position*>);
        static double doCompareFunction(inn::Position*, inn::Position*);
        static double getGammaFunctionValue(double, double, double, double);
        static std::pair<double, double> getFiFunctionValue(double, double, double, double);
        static double getReceptorInfluenceValue(bool, double, inn::Position*, inn::Position*);
        static double getRcValue(double, double, double, double);
        static void getNewPosition(inn::Position*, inn::Position*, inn::Position*, double, double);
        static double getLambdaValue(unsigned int);
        static double getFiVectorLength(double);
        static double getSynapticSensitivityValue(unsigned int, unsigned int);
    };
}

#endif //INTERFERENCE_COMPUTER_H
