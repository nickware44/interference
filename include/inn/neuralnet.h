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
        //typedef std::tuple<int64_t, unsigned int, inn::Neuron*> NeuronDefinition;
        typedef std::multimap<inn::Neuron*, inn::NeuralNet::Link> LinkMap;
        typedef std::pair<inn::NeuralNet::LinkMap::iterator, inn::NeuralNet::LinkMap::iterator> LinkMapRange;
        std::string Name, Description, Version;
        unsigned int EntriesCount;
        unsigned int LDRCounterE, LDRCounterN;
        int64_t t;
        bool DataDone;
        bool Learned;
        std::map<std::string, std::vector<std::string>> Entries;
        std::map<std::string, inn::Neuron*> Neurons;
        std::map<std::string, int> Latencies;
        std::vector<std::string> Outputs;
        //std::vector<inn::NeuralNet::NeuronDefinition> Neurons;
        //inn::NeuralNet::LinkMap NeuronLinks;
        void doAddNeuron(Neuron*, std::vector<inn::LinkDefinition>);
        void doAddNeuron(Neuron*, std::vector<inn::LinkDefinitionRange>);
        void doAddNeuron(Neuron*, inn::LinkDefinitionRangeNext);
        void doCreateNewEntries(unsigned int);
        void doCreateNewOutput(const std::string&);
        void doPrepare();
    public:
        NeuralNet();
        std::vector<double> doComparePatterns();
        void doEnableMultithreading();
        void doFinalize();
        void doReset();
        void doSignalSend(const std::vector<double>&);
        std::vector<double> doSignalReceive();
        bool isMultithreadingEnabled();
        void setStructure(std::ifstream&);
        void setStructure(const std::string &Str);
        std::string getStructure();
        std::string getName();
        std::string getDescription();
        std::string getVersion();
        inn::Neuron* getNeuron(const std::string&);
        uint64_t getNeuronCount();
        ~NeuralNet();
    };

    class NeuralNet::Link {
    private:
        unsigned int Latency;
        inn::LinkType LT;
        int LinkFromEID;
        inn::Neuron *LinkFromE;
        int64_t t;
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
        int64_t getTime();
        ~Link() = default;
    };
}

#endif //INTERFERENCE_NEURALNET_H
