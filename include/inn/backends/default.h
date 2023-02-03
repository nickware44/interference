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
    private:
        inn::Position *dRPos, *nRPos;
    public:
        ComputeBackendDefault();
        void doProcessNeuron(void*);
    };
}

#endif //INTERFERENCE_DEFAULT_H
