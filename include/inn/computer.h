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
        virtual void doRegisterHost(const std::vector<void*>&) = 0;
        virtual void doUnregisterHost() = 0;
        virtual void doWaitTarget() = 0;
        virtual void doProcess(void*) = 0;
        static std::vector<float> doCompareCPFunction(std::vector<inn::Position*>, std::vector<inn::Position*>);
        static float doCompareCPFunctionD(std::vector<inn::Position*>, std::vector<inn::Position*>);
        static float doCompareFunction(inn::Position*, inn::Position*);
        static float getGammaFunctionValue(float, float, float, float);
        static std::pair<float, float> getFiFunctionValue(float, float, float, float);
        static float getReceptorInfluenceValue(bool, float, inn::Position*, inn::Position*);
        static float getRcValue(float, float, float, float);
        static void getNewPosition(inn::Position*, inn::Position*, inn::Position*, float, float);
        static float getLambdaValue(unsigned int);
        static float getFiVectorLength(float);
        static float getSynapticSensitivityValue(unsigned int, unsigned int);
    };
}

#endif //INTERFERENCE_COMPUTER_H
