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

#include <inn/computer.h>

namespace inn {
    class ComputeBackendDefault : public Computer {
    private:
        inn::Position *dRPos, *nRPos, *zPos;
    public:
        ComputeBackendDefault();
        void doRegisterHost(const std::vector<void*>&) override;
        void doUnregisterHost() override;
        void doWaitTarget() override;
        void doProcess(void*) override;
    };
}

#endif //INTERFERENCE_DEFAULT_H
