/////////////////////////////////////////////////////////////////////////////
// Name:        inn/neuralnet.h
// Purpose:     Neural net classes header
// Author:      Nickolay Babbysh
// Created:     12.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_NEURALNET_H
#define INTERFERENCE_NEURALNET_H

#include <map>
#include <algorithm>
#include <tuple>
#include "neuron.h"

namespace inn {
    typedef unsigned int LinkType;
    typedef std::pair<inn::LinkType, unsigned int> LinkDefinition;
    typedef std::tuple<inn::LinkType, unsigned int, unsigned int> LinkDefinitionRange;
    typedef std::tuple<inn::LinkType, unsigned int> LinkDefinitionRangeNext;
    enum {LINK_ENTRY2NEURON, LINK_NEURON2NEURON};

    class NeuralNet {
    private:
        class Link;
        typedef std::tuple<unsigned long long, unsigned int, inn::Neuron*> NeuronDefinition;
        typedef std::multimap<inn::Neuron*, inn::NeuralNet::Link> LinkMap;
        typedef std::pair<inn::NeuralNet::LinkMap::iterator, inn::NeuralNet::LinkMap::iterator> LinkMapRange;
        unsigned int EntriesCount;
        unsigned int LDRCounterE, LDRCounterN;
        unsigned long long t;
        bool DataDone;
        bool Learned;
        std::vector<inn::NeuralNet::NeuronDefinition> Neurons;
        inn::NeuralNet::LinkMap NeuronLinks;
        std::vector<inn::Neuron*> Outputs;
    public:
        NeuralNet();
        void doAddNeuron(Neuron*, std::vector<inn::LinkDefinition>);
        void doAddNeuron(Neuron*, std::vector<inn::LinkDefinitionRange>);
        void doAddNeuron(Neuron*, inn::LinkDefinitionRangeNext);
        void doCreateNewEntries(unsigned int);
        void doCreateNewOutput(unsigned long long);
        std::vector<double> doComparePatterns();
        void doEnableMultithreading();
        void doPrepare();
        void doFinalize();
        void doReinit();
        void doSignalSend(std::vector<double>);
        std::vector<double> doSignalReceive();
        bool isMultithreadingEnabled();
        inn::Neuron* getNeuron(unsigned long long);
        unsigned long long getNeuronCount();
        ~NeuralNet();
    };

    class NeuralNet::Link {
    private:
        unsigned int Latency;
        inn::LinkType LT;
        int LinkFromEID;
        inn::Neuron *LinkFromE;
        unsigned long long t;
    public:
        Link() = default;
        Link(inn::LinkType, int);
        Link(inn::LinkType, inn::Neuron*);
        bool doCheckSignal();
        void doResetSignalController();
        void setLatency(unsigned int);
        inn::LinkType getLinkType();
        unsigned int getLatency();
        int getLinkFromEID();
        inn::Neuron* getLinkFromE();
        double getSignal();
        unsigned long long getTime();
        ~Link() = default;
    };
}

#endif //INTERFERENCE_NEURALNET_H
