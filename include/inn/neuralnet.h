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
    /**
     * Main neural net class.
     */
    class NeuralNet {
    private:
        std::string Name, Description, Version;
        int64_t t;
        bool Learned;
        std::map<std::string, std::vector<std::string>> Entries;
        std::map<std::string, std::vector<std::string>> Ensembles;
        std::vector<std::string> Outputs;
        void doSignalProcessStart();
        std::vector<std::pair<std::queue<std::tuple<std::string, std::string, double, int64_t>>, std::vector<std::string>>> ContextCascade;
        std::map<std::string, inn::Neuron*> Neurons;
        std::map<std::string, int> Latencies;
        inn::Event *DataDoneEvent;
    public:
        NeuralNet();
        std::vector<double> doComparePatterns();
        void doReset();
        void doSignalSend(const std::vector<double>&);
        std::vector<double> doSignalTransfer(const std::vector<std::vector<double>>&);
        void doSignalTransferAsync(const std::vector<std::vector<double>>&, const std::function<void(std::vector<double>)>& Callback = nullptr);
        std::vector<double> doLearn(const std::vector<std::vector<double>>&);
        std::vector<double> doRecognise(const std::vector<std::vector<double>>&);
        void doLearnAsync(const std::vector<std::vector<double>>&, const std::function<void(std::vector<double>)>& Callback = nullptr);
        void doRecogniseAsync(const std::vector<std::vector<double>>&, const std::function<void(std::vector<double>)>& Callback = nullptr);
        std::vector<double> doSignalReceive();
        void doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries = false);
        void setStructure(std::ifstream&);
        void setStructure(const std::string &Str);
        void setLearned(bool LearnedFlag);
        bool isLearned();
        std::string getStructure();
        std::string getName();
        std::string getDescription();
        std::string getVersion();
        std::vector<inn::Neuron*> getEnsemble(const std::string&);
        inn::Neuron* getNeuron(const std::string&);
        uint64_t getNeuronCount();
        ~NeuralNet();
    };
}

#endif //INTERFERENCE_NEURALNET_H
