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
#include <indk/position.h>

namespace indk {
    class Computer {
    public:
        Computer();
        virtual void doRegisterHost(const std::vector<void*>&) = 0;
        virtual void doUnregisterHost() = 0;
        virtual void doWaitTarget() = 0;
        virtual void doProcess(void*) = 0;
        static std::vector<float> doCompareCPFunction(std::vector<indk::Position*>, std::vector<indk::Position*>);
        static float doCompareCPFunctionD(std::vector<indk::Position*>, std::vector<indk::Position*>);
        static float doCompareFunction(indk::Position*, indk::Position*);
        static float getGammaFunctionValue(float, float, float, float);
        static std::pair<float, float> getFiFunctionValue(float, float, float, float);
        static float getReceptorInfluenceValue(bool, float, indk::Position*, indk::Position*);
        static float getRcValue(float, float, float, float);
        static void getNewPosition(indk::Position*, indk::Position*, indk::Position*, float, float);
        static float getLambdaValue(unsigned int);
        static float getFiVectorLength(float);
        static float getSynapticSensitivityValue(unsigned int, unsigned int);
    };
}

#endif //INTERFERENCE_COMPUTER_H
