/////////////////////////////////////////////////////////////////////////////
// Name:        error.cpp
// Purpose:     Exception system class
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <indk/error.h>

indk::Error::Error() {
    ET = 0;
}

indk::Error::Error(ExceptionType _ET) {
    ET = _ET;
}

indk::Error::Error(ExceptionType _ET, std::vector<float> _ED) {
    ET = _ET;
	ED = std::move(_ED);
}

const char* indk::Error::what() const noexcept {
    std::string Msg;
    char *B;
    switch (ET) {
        case EX_NEURALNET_NEURONS:
            Msg = std::string("EX_NEURALNET_NEURONS ~ Out of neuron list");
            break;
        case EX_NEURALNET_INPUT:
            Msg = std::string("EX_NEURALNET_INPUT ~ The number of input signals does not match the neural net entries count");
            break;
        case EX_NEURALNET_ENTRIES:
            Msg = std::string("EX_NEURALNET_ENTRIES ~ Out of neural net entries list");
            break;
        case EX_NEURALNET_NEURON_ENTRIES:
            Msg = std::string("EX_NEURALNET_NEURON_ENTRIES ~ The number of links more than the neuron entries count");
            break;
        case EX_NEURALNET_LINKTYPE:
            Msg = std::string("EX_NEURALNET_LINKTYPE ~ Unknown link type");
            break;
        case EX_NEURON_INPUT:
            Msg = std::string("EX_NEURON_INPUT ~ The number of input signals does not match the neuron entries count");
            break;
        case EX_NEURON_ENTRIES:
            Msg = std::string("EX_NEURON_ENTRIES ~ Out of entry list");
            break;
        case EX_NEURON_RECEPTORS:
            Msg = std::string("EX_NEURON_RECEPTORS ~ Out of receptor list");
            break;
        case EX_POSITION_OUT_RANGES:
			Msg = std::string("EX_POSITION_OUT_RANGES ~ Coordinates out of range ("+std::to_string(ED[0])+" < 0 || "+std::to_string(ED[0])+" > "+std::to_string(ED[1])+")");
            break;
        case EX_POSITION_RANGES:
            Msg = std::string("EX_POSITION_RANGES ~ Not equal coordinates ranges");
            break;
        case EX_POSITION_DIMENSIONS:
            Msg = std::string("EX_POSITION_DIMENSIONS ~ Not equal space dimensions of positions");
            break;
        default:
            Msg = std::string("No exception");
    }
    auto *S = new char[Msg.size()];
    sprintf(S, "%s", Msg.c_str());
    return S;
}
