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
#include <unordered_map>
#include "neuron.h"
#include "../../include/inn/system.h"

namespace inn {
    typedef std::queue<std::tuple<std::string, std::string, void*, int64_t>> NQueue;

    typedef std::tuple<std::string, std::string, void*, void*, int> LinkDefinition;
    typedef std::vector<LinkDefinition> NList;

    /**
     * Main neural net class.
     */
    class NeuralNet {
    private:
        std::string Name, Description, Version;
        int64_t t;
        bool Learned;
        std::vector<std::pair<std::string, std::vector<std::string>>> Entries;
        std::map<std::string, std::vector<std::string>> Ensembles;
        std::vector<std::string> Outputs;

        int64_t doFindEntry(const std::string&);
        void doSignalProcessStart(const inn::NList&, const std::vector<std::vector<float>>&);
        std::vector<std::pair<std::queue<std::tuple<std::string, std::string, float, int64_t>>, std::vector<std::string>>> ContextCascade;
        std::map<std::string, inn::Neuron*> Neurons;
        std::map<std::string, int> Latencies;

        NList Links;
        bool Prepared;

        int LastUsedComputeBackend;
    public:
        NeuralNet();
        std::vector<float> doComparePatterns();
        void doReset();
        void doStructurePrepare();
        std::vector<float> doSignalTransfer(const std::vector<std::vector<float>>&);
        void doSignalTransferAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr);
        std::vector<float> doLearn(const std::vector<std::vector<float>>&);
        std::vector<float> doRecognise(const std::vector<std::vector<float>>&);
        void doLearnAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr);
        void doRecogniseAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr);
        std::vector<float> doSignalReceive();
        void doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries = false);
        void doReserveSignalBuffer(int64_t L);
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
        int64_t getSignalBufferSize();
        ~NeuralNet();
    };
}

#endif //INTERFERENCE_NEURALNET_H
