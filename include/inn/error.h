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
    /// Error handler class.
    class Error: public std::exception {
    public:
        typedef unsigned int ExceptionType;
        /**
         * Exception types.
         */
        typedef enum {
            /// Out of neuron list.
            EX_NEURALNET_NEURONS,
            /// The number of input signals does not match the neural net entries count.
            EX_NEURALNET_INPUT,
            /// Out of neural net entries list.
            EX_NEURALNET_ENTRIES,
            /// The number of links more than the neuron entries count.
            EX_NEURALNET_NEURON_ENTRIES,
            EX_NEURALNET_LINKTYPE,
            /// The number of input signals does not match the neuron entries count.
            EX_NEURON_INPUT,
            /// Out of entry list.
            EX_NEURON_ENTRIES,
            /// Out of receptor list.
            EX_NEURON_RECEPTORS,
            /// Coordinates out of range.
            EX_POSITION_OUT_RANGES,
            /// Not equal coordinates ranges.
            EX_POSITION_RANGES,
            /// Not equal space dimensions of positions.
            EX_POSITION_DIMENSIONS
        } Exceptions;

        Error();
        explicit Error(ExceptionType);
        explicit Error(ExceptionType, std::vector<double>);
        const char* what() const noexcept override;
    private:
        ExceptionType ET;
        std::vector<double> ED;
    };
}

#endif //INTERFERENCE_ERROR_H
