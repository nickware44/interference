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
#include "neuron.h"

namespace inn {
    typedef unsigned int LinkType;
    typedef std::pair<inn::LinkType, unsigned int> LinkDefinition;
    enum {LINK_ENTRY2NEURON, LINK_NEURON2NEURON};

    class NeuralNet {
    private:
        class Link;
        typedef std::multimap<inn::Neuron*, inn::NeuralNet::Link> LinkMap;
        typedef std::pair<inn::NeuralNet::LinkMap::iterator, inn::NeuralNet::LinkMap::iterator> LinkMapRange;
        unsigned int EntriesCount;
        std::vector<std::tuple<unsigned int, unsigned int, inn::Neuron*> > Neurons;
        inn::NeuralNet::LinkMap NeuronLinks;
        std::vector<inn::Neuron*> Outputs;
    public:
        NeuralNet();
        void doAddNeuron(Neuron*, std::vector<LinkDefinition>);
        void doCreateNewEntries(unsigned int);
        void doCreateNewOutput(unsigned int);
        void doPrepare();
        void doSignalSend(std::vector<double>);
        std::vector<double> doSignalReceive();
        inn::Neuron* getNeuron(unsigned int);
        ~NeuralNet() = default;
    };

    class NeuralNet::Link {
    private:
        unsigned int Latency;
        inn::LinkType LT;
        int LinkFromEID;
        inn::Neuron *LinkFromE;
    public:
        Link() = default;
        Link(inn::LinkType, int);
        Link(inn::LinkType, inn::Neuron*);
        void setLatency(unsigned int);
        inn::LinkType getLinkType();
        unsigned int getLatency();
        int getLinkFromEID();
        inn::Neuron* getLinkFromE();
        ~Link() = default;
    };
}

#endif //INTERFERENCE_NEURALNET_H
