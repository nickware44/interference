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

namespace inn {
    class Computer {
    private:
        std::queue<std::vector<double>> DataQueue;
    public:
        Computer();
        void doAddData();
        void doProcess();
        ~Computer() = default;
    };
}

#endif //INTERFERENCE_COMPUTER_H
