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
    enum {ComputerBackendDefault, ComputerBackendMultithread};

    class NeuralNet {
    private:
        int CurrentBackend;
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
        void doPrepare();
    public:
        NeuralNet();
        std::vector<double> doComparePatterns();
        void doFinalize();
        void doReset();
        void doSignalSend(const std::vector<double>&);
        std::vector<double> doSignalReceive();
        bool isMultithreadingEnabled();
        void setStructure(std::ifstream&);
        void setStructure(const std::string &Str);
        void setComputerBackend(int);
        std::string getStructure();
        int getComputerBackend() const;
        std::string getName();
        std::string getDescription();
        std::string getVersion();
        inn::Neuron* getNeuron(const std::string&);
        uint64_t getNeuronCount();
        ~NeuralNet();
    };
}

#endif //INTERFERENCE_NEURALNET_H
