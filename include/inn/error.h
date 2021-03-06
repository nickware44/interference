/////////////////////////////////////////////////////////////////////////////
// Name:        inn/error.h
// Purpose:     Exception system class header
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_ERROR_H
#define INTERFERENCE_ERROR_H

#include <iostream>
#include <exception>
#include <vector>
#include <string>

namespace inn {
    typedef unsigned int ExceptionType;
    enum {EX_NEURALNET_NEURONS, EX_NEURALNET_INPUT, EX_NEURALNET_ENTRIES, EX_NEURALNET_NEURON_ENTRIES,
            EX_NEURALNET_LINKTYPE, EX_NEURON_INPUT, EX_NEURON_ENTRIES, EX_NEURON_RECEPTORS, EX_POSITION_OUT_RANGES, EX_POSITION_RANGES, EX_POSITION_DIMENSIONS};

    class Error: public std::exception {
    private:
        ExceptionType ET;
        std::vector<double> ED;
    public:
        Error();
        explicit Error(ExceptionType);
        explicit Error(ExceptionType, std::vector<double>);
        const char* what() const noexcept override;
    };
}

#endif //INTERFERENCE_ERROR_H
