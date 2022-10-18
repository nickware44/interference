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
    private:
        std::queue<std::vector<double>> DataQueue;
    public:
        Computer();
        virtual std::vector<double> doCompareCPFunction(std::vector<inn::Position*>, std::vector<inn::Position*>) = 0;
        virtual double doCompareCPFunctionD(std::vector<inn::Position*>, std::vector<inn::Position*>) = 0;
        virtual double doCompareFunction(inn::Position*, inn::Position*) = 0;
        virtual double getGammaFunctionValue(double, double, double, double, double) = 0;
        virtual std::pair<double, double> getFiFunctionValue(double, double, double, double) = 0;
        virtual double getReceptorInfluenceValue(bool, double, inn::Position*, inn::Position*) = 0;
        virtual double getRcValue(double, double, double, double) = 0;
        virtual void getNewPosition(inn::Position*, inn::Position*, inn::Position*, double, double) = 0;
        virtual int64_t getOutputSignalQMaxSizeValue(unsigned int) = 0;
        virtual int64_t getGammaQMaxSizeValue(double) = 0;
        virtual double getLambdaValue(unsigned int) = 0;
        virtual double getFiVectorLength(double) = 0;
        virtual double getSynapticSensitivityValue(unsigned int, unsigned int) = 0;
        void doAddData();
        void doProcess();
    };
}

#endif //INTERFERENCE_COMPUTER_H
