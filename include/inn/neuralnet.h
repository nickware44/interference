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
#include <functional>
#include "neuron.h"
#include "../../include/inn/system.h"

namespace inn {
    class NeuralNet {
    private:
        std::string Name, Description, Version;
        int64_t t;
        unsigned int EntriesCount;
        unsigned int LDRCounterE, LDRCounterN;
        bool DataDone;
        bool Learned;
        std::map<std::string, std::vector<std::string>> Entries;
        std::vector<std::string> Outputs;
        void doPrepare();
        void doSignalProcessStart();
    public:
        NeuralNet();
        std::vector<double> doComparePatterns();
        void doFinalize();
        void doReset();
        void doSignalSend(const std::vector<double>&);
        std::vector<double> doSignalTransfer(const std::vector<std::vector<double>>&);
        void doSignalTransferAsync(const std::vector<std::vector<double>>&, const std::function<void(std::vector<double>)> Callback = nullptr);
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
}

#endif //INTERFERENCE_NEURALNET_H
