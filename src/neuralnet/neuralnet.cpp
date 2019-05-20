/////////////////////////////////////////////////////////////////////////////
// Name:        neuralnet/neuralnet.cpp
// Purpose:     Neural net main class
// Author:      Nickolay Babbysh
// Created:     12.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include "../../include/inn/neuralnet.h"
#include "../../include/inn/error.h"

inn::NeuralNet::NeuralNet() {
    EntriesCount = 0;
}

void inn::NeuralNet::doAddNeuron(Neuron *N, std::vector<inn::LinkDefinition> LinkFromTo) {
    int i = 0;
    unsigned int ML, MLF = 0;
    inn::NeuralNet::LinkMapRange Range;
    for (auto L: LinkFromTo) {
        if (i >= N->getEntriesCount()) {
            throw inn::Error(inn::EX_NEURALNET_NEURON_ENTRIES);
        }
        if (std::get<0>(L) == LINK_ENTRY2NEURON && std::get<1>(L) >= EntriesCount) {
            throw inn::Error(inn::EX_NEURALNET_ENTRIES);
        }
        switch (std::get<0>(L)) {
            case LINK_ENTRY2NEURON:
                NeuronLinks.insert(std::pair<inn::Neuron*, inn::NeuralNet::Link>(N, inn::NeuralNet::Link(std::get<0>(L), std::get<1>(L))));
                break;
            case LINK_NEURON2NEURON:
                ML = 0;
                NeuronLinks.insert(std::pair<inn::Neuron*, inn::NeuralNet::Link>(N, inn::NeuralNet::Link(std::get<0>(L), std::get<2>(Neurons[std::get<1>(L)]))));
                Range = NeuronLinks.equal_range(std::get<2>(Neurons[std::get<1>(L)]));
                for (auto it = Range.first; it != Range.second; ++it)
                    if (it->second.getLatency() > ML) ML = it->second.getLatency();
                if (ML > MLF) MLF = ML;
                NeuronLinks.end()->second.setLatency(ML+1);
                break;
            default:
                throw inn::Error(inn::EX_NEURALNET_LINKTYPE);
        }
        i++;
    }
    Neurons.emplace_back(Neurons.size(), ML, N);
}

void inn::NeuralNet::doCreateNewEntries(unsigned int _EC) {
    EntriesCount = _EC;
}

void inn::NeuralNet::doCreateNewOutput(unsigned int NID) {
    if (NID >= Neurons.size()) {
        throw inn::Error(inn::EX_NEURALNET_NEURONS);
    }
    Outputs.push_back(std::get<2>(Neurons[NID]));
}

void inn::NeuralNet::doPrepare() {
    std::sort(Neurons.begin(), Neurons.end(), [](const std::tuple<unsigned int, unsigned int, inn::Neuron*> &N1, const std::tuple<unsigned int, unsigned int, inn::Neuron*> &N2) -> bool
    {
        return std::get<1>(N1) > std::get<1>(N2);
    });
}

void inn::NeuralNet::doSignalSend(std::vector<double> X) {
    std::vector<double> nX;
    for (auto N: Neurons) {
        LinkMapRange R = NeuronLinks.equal_range(std::get<2>(N));
        for (auto it = R.first; it != R.second; ++it) {
            if (it->second.getLinkType() == LINK_ENTRY2NEURON) {
                if (EntriesCount != X.size()) {
                    throw inn::Error(inn::EX_NEURALNET_INPUT);
                }
                nX.push_back(X[it->second.getLinkFromEID()]);
            }
            else nX.push_back(it->second.getLinkFromE()->doSignalReceive());
        }
        std::get<2>(N) -> doSignalsSend(nX);
        nX.clear();
    }
}

std::vector<double> inn::NeuralNet::doSignalReceive() {
    std::vector<double> nY;
    for (auto N: Outputs) nY.push_back(N->doSignalReceive());
    return nY;
}

inn::Neuron* inn::NeuralNet::getNeuron(unsigned int NID) {
    if (NID >= Neurons.size()) {
        throw inn::Error(inn::EX_NEURALNET_NEURONS);
    }
    for (auto N: Neurons) if (std::get<0>(N) == NID) return std::get<2>(N);
}
