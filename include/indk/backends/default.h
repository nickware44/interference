/////////////////////////////////////////////////////////////////////////////
// Name:
// Purpose:
// Author:      Nickolay Babbysh
// Created:     18.10.22
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////
#ifndef INTERFERENCE_DEFAULT_H
#define INTERFERENCE_DEFAULT_H

#include <indk/computer.h>

namespace indk {
    class ComputeBackendDefault : public Computer {
    private:
        indk::Position *dRPos, *nRPos, *zPos;
    public:
        ComputeBackendDefault();
        void doRegisterHost(const std::vector<void*>&) override;
        void doUnregisterHost() override;
        void doWaitTarget() override;
        void doProcess(void*) override;
    };
}

#endif //INTERFERENCE_DEFAULT_H
