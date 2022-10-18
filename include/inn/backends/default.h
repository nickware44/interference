/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_DEFAULT_H
#define INTERFERENCE_DEFAULT_H

#include "../computer.h"

namespace inn {
    class ComputeBackendDefault : public Computer {
    public:
        std::vector<double> doCompareCPFunction(std::vector<inn::Position*>, std::vector<inn::Position*>) override;
        double doCompareCPFunctionD(std::vector<inn::Position*>, std::vector<inn::Position*>) override;
        double doCompareFunction(inn::Position*, inn::Position*) override;
        double getGammaFunctionValue(double, double, double, double, double) override;
        std::pair<double, double> getFiFunctionValue(double, double, double, double) override;
        double getReceptorInfluenceValue(bool, double, inn::Position*, inn::Position*) override;
        double getRcValue(double, double, double, double) override;
        void getNewPosition(inn::Position*, inn::Position*, inn::Position*, double, double) override;
        int64_t getOutputSignalQMaxSizeValue(unsigned int) override;
        int64_t getGammaQMaxSizeValue(double) override;
        double getLambdaValue(unsigned int) override;
        double getFiVectorLength(double) override;
        double getSynapticSensitivityValue(unsigned int, unsigned int) override;
    };
}

#endif //INTERFERENCE_DEFAULT_H
