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
    LDRCounterE = 0;
    LDRCounterN = 0;
    t = 0;
    DataDone = false;
}

void inn::NeuralNet::doAddNeuron(Neuron *N, std::vector<inn::LinkDefinition> LinkFromTo) {
    int i = 0;
    unsigned int ML, MLF = 0;
    inn::NeuralNet::LinkMapRange Range;
    inn::NeuralNet::Link *nL;
    inn::Neuron *oN;
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
                if (std::get<1>(L) == Neurons.size()) oN = N;
                else oN = std::get<2>(Neurons[std::get<1>(L)]);
                Range = NeuronLinks.equal_range(oN);
                for (auto it = Range.first; it != Range.second; ++it)
                    if (it->second.getLatency() > ML) ML = it -> second.getLatency();
                if (ML+1 > MLF) MLF = ML + 1;
                nL = new inn::NeuralNet::Link(std::get<0>(L), oN);
                nL -> setLatency(ML+1);
                NeuronLinks.insert(std::pair<inn::Neuron*, inn::NeuralNet::Link>(N, *nL));

                break;
            default:
                throw inn::Error(inn::EX_NEURALNET_LINKTYPE);
        }
        i++;
    }
    Neurons.emplace_back(Neurons.size(), MLF, N);
}

void inn::NeuralNet::doAddNeuron(Neuron *N, std::vector<inn::LinkDefinitionRange> LinkFromToRange) {
    std::vector<inn::LinkDefinition> LinkFromTo;
    for (auto &LinkRange: LinkFromToRange) {
        for (auto i = std::get<1>(LinkRange); i <= std::get<2>(LinkRange); i++)
            LinkFromTo.emplace_back(std::get<0>(LinkRange), i);
    }
    doAddNeuron(N, LinkFromTo);
}

void inn::NeuralNet::doAddNeuron(Neuron *N, inn::LinkDefinitionRangeNext LinkFromToRangeNext) {
    std::vector<inn::LinkDefinition> LinkFromTo;
    switch (std::get<0>(LinkFromToRangeNext)) {
        case LINK_ENTRY2NEURON:
            for (auto i = LDRCounterE; i < LDRCounterE+std::get<1>(LinkFromToRangeNext); i++)
                LinkFromTo.emplace_back(std::get<0>(LinkFromToRangeNext), i);
            LDRCounterE += std::get<1>(LinkFromToRangeNext);
            break;
        case LINK_NEURON2NEURON:
            for (auto i = LDRCounterN; i < LDRCounterN+std::get<1>(LinkFromToRangeNext); i++)
                LinkFromTo.emplace_back(std::get<0>(LinkFromToRangeNext), i);
            LDRCounterN += std::get<1>(LinkFromToRangeNext);
            break;
        default:
            throw inn::Error(inn::EX_NEURALNET_LINKTYPE);
    }
    doAddNeuron(N, LinkFromTo);
}

void inn::NeuralNet::doCreateNewEntries(unsigned int _EC) {
    EntriesCount = _EC;
}

void inn::NeuralNet::doCreateNewOutput(unsigned long long NID) {
    if (NID >= Neurons.size()) {
        throw inn::Error(inn::EX_NEURALNET_NEURONS);
    }
    Outputs.push_back(std::get<2>(Neurons[NID]));
}

std::vector<double> inn::NeuralNet::doComparePatterns() {
    std::vector<double> PDiff;
    for (auto O: Outputs) PDiff.push_back(O->doComparePattern());
    return PDiff;
}

void inn::NeuralNet::doEnableMultithreading() {
    for (auto N: Neurons) std::get<2>(N) -> doEnableMultithreading();
}

void inn::NeuralNet::doPrepare() {
    std::sort(Neurons.begin(), Neurons.end(), [](const inn::NeuralNet::NeuronDefinition &N1, const inn::NeuralNet::NeuronDefinition &N2) -> bool
    {
        return std::get<1>(N1) > std::get<1>(N2);
    });
    for (auto N: Neurons) std::get<2>(N) -> doPrepare();
}

void inn::NeuralNet::doFinalize() {
    if (Neurons.empty()) return;
    std::vector<double> nX;
    unsigned int Tl = std::get<1>(Neurons[0]);
    if (Tl) {
        for (auto i = 0; i < EntriesCount; i++) nX.push_back(0);
        for (auto i = 0; i < Tl; i++) doSignalSend(nX);
        DataDone = true;
        while (true) {
            bool End = true;
            for (auto N: Neurons) {
                if (std::get<1>(N) == Tl) {
                    LinkMapRange R = NeuronLinks.equal_range(std::get<2>(N));
                    for (auto it = R.first; it != R.second; ++it) {
                        if (it->second.getTime() != t) {
                            End = false;
                            break;
                        }
                    }
                    if (!End) break;
                }
            }
            if (End) break;
            doSignalSend(nX);
        }
    }
    for (auto N: Neurons) std::get<2>(N) -> doFinalize();
}

void inn::NeuralNet::doReinit() {
    t = 0;
    DataDone = false;
    for (auto &NL: NeuronLinks) NL.second.doResetSignalController();
    for (auto N: Neurons) std::get<2>(N) -> doReinit();
}

void inn::NeuralNet::doSignalSend(std::vector<double> X) {
    if (EntriesCount != X.size()) {
        throw inn::Error(inn::EX_NEURALNET_INPUT);
    }
    for (auto N: Neurons) {
        LinkMapRange R = NeuronLinks.equal_range(std::get<2>(N));
        unsigned long long i = 0;
        for (auto it = R.first; it != R.second; ++it, i++) {
            if (it->second.getLinkType() == inn::LINK_ENTRY2NEURON) {
                if (DataDone) continue;
                std::get<2>(N) -> doSignalSendEntry(i, X[it->second.getLinkFromEID()]);
            } else {
                if (!it->second.doCheckSignal()) continue;
                std::get<2>(N) -> doSignalSendEntry(i, it->second.getSignal());
            }
        }
        if (!std::get<2>(N)->isMultithreadingEnabled()) std::get<2>(N) -> doSignalsSend();
    }
    if (!DataDone) t++;
}

std::vector<double> inn::NeuralNet::doSignalReceive() {
    std::vector<double> nY;
    for (auto N: Outputs) nY.push_back(N->doSignalReceive());
    return nY;
}

bool inn::NeuralNet::isMultithreadingEnabled() {
    for (auto N: Neurons) if (!std::get<2>(N)->isMultithreadingEnabled()) return false;
    return true;
}

inn::Neuron* inn::NeuralNet::getNeuron(unsigned long long NID) {
    if (NID >= Neurons.size()) {
        throw inn::Error(inn::EX_NEURALNET_NEURONS);
    }
    for (auto N: Neurons) if (std::get<0>(N) == NID) return std::get<2>(N);
    return nullptr;
}

unsigned long long inn::NeuralNet::getNeuronCount() {
    return Neurons.size();
}

inn::NeuralNet::~NeuralNet() {
    for (auto N: Neurons) delete std::get<2>(N);
}
