/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author: Nickolay Babbysh
// Created: 18.10.22
// Copyright: (c) NickWare Group
// Licence: MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../include/inn/system.h"
#include "../include/inn/backends/default.h"
#include "../include/inn/backends/multithread.h"

int inn::CurrentComputeBackend;
inn::Computer *inn::ComputeBackend;

void inn::setComputeBackend(int Backend, int Parameter) {
    CurrentComputeBackend = Backend;
    delete ComputeBackend;

    switch (CurrentComputeBackend) {
        case inn::ComputeBackends::Default:
            ComputeBackend = new inn::ComputeBackendDefault();
            break;
        case inn::ComputeBackends::Multithread:
            ComputeBackend = new inn::ComputeBackendMultithread(Parameter?Parameter:INN_MULTITHREAD_DEFAULT_NUM);
            break;
    }
}

int inn::getComputeBackend() {
    return CurrentComputeBackend;
}