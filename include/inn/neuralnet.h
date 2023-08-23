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
#include <inn/neuron.h>
#include <inn/system.h>
#include <inn/interlink.h>

namespace inn {
    typedef enum {
        CompareDefault,
        CompareNormalized
    } PatternCompareFlags;

    typedef std::queue<std::tuple<std::string, std::string, void*, int64_t>> NQueue;
    typedef std::vector<std::pair<std::string, std::vector<std::string>>> EntryList;

    /**
     * Main neural net class.
     */
    class NeuralNet {
    private:
        std::string Name, Description, Version;
        int64_t t;

        EntryList Entries;
        std::map<std::string, std::vector<std::string>> Ensembles;
        std::map<std::string, inn::Neuron*> Neurons;
        std::map<std::string, int> Latencies;
        std::vector<std::string> Outputs;

        std::map<std::string, std::vector<std::string>> StateSyncList;

        int64_t doFindEntry(const std::string&);
        void doParseLinks(const EntryList&, const std::string&);
        void doSignalProcessStart(const std::vector<std::vector<float>>&, const EntryList&);
        void doSyncNeuronStates(const std::string&);

        inn::LinkList Links;
        std::string PrepareID;
        bool StateSyncEnabled;
        int LastUsedComputeBackend;

        inn::Interlink *InterlinkService;
        void doInterlinkAppUpdateData();
    public:
        NeuralNet();
        explicit NeuralNet(const std::string &path);
        void doInterlinkInit(int);
        std::vector<float> doComparePatterns(int CompareFlag = inn::PatternCompareFlags::CompareDefault,
                                             int ProcessingMethod = inn::ScopeProcessingMethods::ProcessMin);
        void doCreateNewScope();
        void doChangeScope(uint64_t);
        void doReset();
        void doPrepare();
        void doStructurePrepare();
        std::vector<float> doSignalTransfer(const std::vector<std::vector<float>>&, const std::string& ensemble = "");
        void doSignalTransferAsync(const std::vector<std::vector<float>>&, const std::string& ensemble = "", const std::function<void(std::vector<float>)>& Callback = nullptr);
        std::vector<float> doLearn(const std::vector<std::vector<float>>&);
        std::vector<float> doRecognise(const std::vector<std::vector<float>>&, const std::string& ensemble = "");
        void doLearnAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr);
        void doRecogniseAsync(const std::vector<std::vector<float>>&, const std::string& ensemble, const std::function<void(std::vector<float>)>& Callback = nullptr);
        std::vector<float> doSignalReceive();
        void doReplicateEnsemble(const std::string& From, const std::string& To, bool CopyEntries = false);
        void doReserveSignalBuffer(int64_t L);
        void setStructure(std::ifstream&);
        void setStructure(const std::string &Str);
        void setLearned(bool);
        void setStateSyncEnabled(bool enabled = true);
        bool isLearned();
        std::string getStructure(bool minimized = true);
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
