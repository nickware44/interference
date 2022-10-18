/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_SYSTEM_H
#define INTERFERENCE_SYSTEM_H

#include "computer.h"

namespace inn {
    typedef enum {Default,
                  Multithread,
                  OpenCLGPU} ComputeBackends;

    extern int CurrentComputeBackend;
    extern inn::Computer *ComputeBackend;

    void setComputeBackend(int Backend, int Parameter = 0);
    int getComputeBackend();
}

#endif //INTERFERENCE_SYSTEM_H
