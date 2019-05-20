/////////////////////////////////////////////////////////////////////////////
// Name:        inn/error.h
// Purpose:     Error class header
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_ERROR_H
#define INTERFERENCE_ERROR_H

#include <iostream>
#include <exception>

namespace inn {
    typedef unsigned int ExceptionType;
    enum {EX_NEURALNET_NEURONS, EX_NEURALNET_INPUT, EX_NEURALNET_ENTRIES, EX_NEURALNET_NEURON_ENTRIES,
            EX_NEURALNET_LINKTYPE, EX_NEURON_INPUT, EX_POSITION_RANGES, EX_POSITION_DIMENSIONS};

    class Error {
    private:
        ExceptionType ET;
    public:
        Error();
        explicit Error(ExceptionType);
        const char* getInfo();
        ~Error() = default;
    };
}

#endif //INTERFERENCE_ERROR_H
