/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_MULTITHREAD_H
#define INTERFERENCE_MULTITHREAD_H

#include <thread>
#include "../computer.h"

#define INN_MULTITHREAD_DEFAULT_NUM 2

namespace inn {
    class ComputeBackendMultithread : public Computer {
    private:
        std::vector<std::thread> Workers;
        std::queue<double> DataQueue;
        static void tWorker(int);
    public:
        explicit ComputeBackendMultithread(int);
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

#endif //INTERFERENCE_MULTITHREAD_H
