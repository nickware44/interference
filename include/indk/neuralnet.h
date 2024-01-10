/////////////////////////////////////////////////////////////////////////////
// Name:        indk/neuralnet.h
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
#include <indk/neuron.h>
#include <indk/system.h>
#include <indk/interlink.h>

namespace indk {
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
        std::map<std::string, indk::Neuron*> Neurons;
        std::map<std::string, int> Latencies;
        std::vector<std::string> Outputs;

        std::map<std::string, std::vector<std::string>> StateSyncList;

        int64_t doFindEntry(const std::string&);
        void doParseLinks(const EntryList&, const std::string&);
        void doSignalProcessStart(const std::vector<std::vector<float>>&, const EntryList&);
        void doSyncNeuronStates(const std::string&);

        indk::LinkList Links;
        std::string PrepareID;
        bool StateSyncEnabled;
        int LastUsedComputeBackend;

        indk::Interlink *InterlinkService;
        std::vector<std::vector<std::string>> InterlinkDataBuffer;

    public:
        NeuralNet();
        explicit NeuralNet(const std::string &path);
        void doInterlinkInit(int port, int timeout = 5);
        void doInterlinkSyncStructure();
        void doInterlinkSyncData();
        std::vector<float> doComparePatterns(int CompareFlag = indk::PatternCompareFlags::CompareDefault,
                                             int ProcessingMethod = indk::ScopeProcessingMethods::ProcessMin,
                                             std::vector<std::string> nnames = {});
        void doCreateNewScope();
        void doChangeScope(uint64_t);
        void doAddNewOutput(const std::string&);
        void doIncludeNeuronToEnsemble(const std::string&, const std::string&);
        void doReset();
        void doPrepare();
        void doStructurePrepare();
        std::vector<float> doSignalTransfer(const std::vector<std::vector<float>>& X, const std::vector<std::string>& inputs = {});
        void doSignalTransferAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr, const std::vector<std::string>& inputs = {});
        std::vector<float> doLearn(const std::vector<std::vector<float>>&, bool prepare = true, const std::vector<std::string>& inputs = {});
        std::vector<float> doRecognise(const std::vector<std::vector<float>>&, bool prepare = true, const std::vector<std::string>& inputs = {});
        void doLearnAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr, bool prepare = true, const std::vector<std::string>& inputs = {});
        void doRecogniseAsync(const std::vector<std::vector<float>>&, const std::function<void(std::vector<float>)>& Callback = nullptr, bool prepare = true, const std::vector<std::string>& inputs = {});
        std::vector<float> doSignalReceive();
        void doReplicateNeuron(const std::string& from, const std::string& to, bool integrate);
        void doDeleteNeuron(const std::string& name);
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
        std::vector<indk::Neuron*> getEnsemble(const std::string&);
        indk::Neuron* getNeuron(const std::string&);
        std::vector<indk::Neuron*> getNeurons();
        uint64_t getNeuronCount();
        int64_t getSignalBufferSize();
        ~NeuralNet();
    };
}

#endif //INTERFERENCE_NEURALNET_H
