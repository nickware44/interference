/////////////////////////////////////////////////////////////////////////////
// Name:        indk/neuron.h
// Purpose:     Neuron classes header
// Author:      Nickolay Babbysh
// Created:     29.04.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_NEURON_H
#define INTERFERENCE_NEURON_H

#include <cmath>
#include <vector>
#include <atomic>
#include <map>
#include <iostream>
#include <indk/position.h>

namespace indk {
    typedef enum {
        ProcessMin,
        ProcessAverage,
    } ScopeProcessingMethods;

    typedef unsigned int TopologyID;

    class Neuron {
    private:
        class Entry;
        class Synapse;
        class Receptor;
        std::vector<std::pair<std::string, indk::Neuron::Entry*>> Entries;
        std::vector<std::string> Links;
        std::vector<indk::Neuron::Receptor*> Receptors;
        std::atomic<int64_t> t;
        int64_t Tlo;
        unsigned int Xm, DimensionsCount;
        float *OutputSignal;
        int64_t OutputSignalSize;
        int64_t OutputSignalPointer;
        int NID, OutputMode;
        bool Learned;
        std::vector<float> doCompareCheckpoints();
        std::string Name;
    public:
        /**
         * Neuron states.
         */
        typedef enum {
            /// Neuron not processed (initial state).
            NotProcessed,
            /// Neuron is processing now.
            Pending,
            /// Processing of neuron is done.
            Computed,
        } States;
        typedef std::tuple<float, float> PatternDefinition;
        Neuron();
        Neuron(const indk::Neuron&);
        Neuron(unsigned int, unsigned int, int64_t, const std::vector<std::string>& InputSignals);
        void doCreateNewSynapse(const std::string&, std::vector<float>, float, int64_t, int);
        void doCreateNewSynapseCluster(const std::vector<float>& PosVector, unsigned R, float k1, int64_t Tl, int NT);
        void doCreateNewReceptor(std::vector<float>);
        void doCreateNewReceptorCluster(const std::vector<float>& PosVector, unsigned R, unsigned C);
        bool doSignalSendEntry(const std::string&, float, int64_t);
        std::pair<int64_t, float> doSignalReceive(int64_t tT = -1);
        void doFinalizeInput(float);
        void doPrepare();
        void doFinalize();
        void doCreateNewScope();
        void doChangeScope(uint64_t);
        void doReset();
        void doCreateCheckpoint();
        indk::Neuron::PatternDefinition doComparePattern(int ProcessingMethod = indk::ScopeProcessingMethods::ProcessMin) const;
        void doLinkOutput(const std::string&);
        void doClearOutputLinks();
        void doClearEntries();
        void doAddEntryName(const std::string&);
        void doCopyEntry(const std::string&, const std::string&);
        void doReplaceEntryName(const std::string&, const std::string&);
        void doReserveSignalBuffer(int64_t);
        void setTime(int64_t);
        void setEntries(const std::vector<std::string>& inputs);
        void setLambda(float);
        void setk1(float);
        void setk2(float);
        void setk3(float);
        void setNID(int);
        void setName(const std::string&);
        void setLearned(bool LearnedFlag);
        bool isLearned() const;
        std::vector<std::string> getLinkOutput() const;
        std::vector<std::string> getEntries() const;
        indk::Neuron::Entry*  getEntry(int64_t) const;
        indk::Neuron::Receptor* getReceptor(int64_t) const;
        std::vector<std::string> getWaitingEntries();
        int64_t getEntriesCount() const;
        unsigned int getSynapsesCount() const;
        int64_t getReceptorsCount() const;
        int64_t getTime() const;
        unsigned int getXm() const;
        unsigned int getDimensionsCount() const;
        int64_t getTlo() const;
        int getNID() const;
        std::string getName();
        int64_t getSignalBufferSize() const;
        int getState(int64_t) const;
        ~Neuron();
    };

    class Neuron::Entry {
    private:
        std::vector<indk::Neuron::Synapse*> Synapses;
        int64_t t, tm;
        float *Signal;
        int64_t SignalSize;
        int64_t SignalPointer;
    public:
        Entry();
        Entry(const indk::Neuron::Entry&);
        bool doCheckState(int64_t) const;
        void doAddSynapse(indk::Position*, unsigned int, float, int64_t, int);
        void doIn(float, int64_t);
        void doProcess();
        void doPrepare();
        void doFinalize();
        void doReserveSignalBuffer(uint64_t);
        void setLambda(float);
        void setk1(float);
        void setk2(float);
        indk::Neuron::Synapse* getSynapse(int64_t) const;
        int64_t getSynapsesCount() const;
        float getIn();
        ~Entry();
    };

    class Neuron::Synapse {
    private:
        indk::Position* SPos;
        float ok1, ok2, k1, k2;
        float Lambda;
        int NeurotransmitterType;
        int64_t Tl;
        float Gamma, dGamma;
        long long QCounter;
        std::vector<float> GammaQ;
        std::atomic<int64_t> QSize;
    public:
        Synapse();
        Synapse(const indk::Neuron::Synapse&);
        Synapse(indk::Position*, float, float, int64_t, int);
        void doIn(float);
        void doSendToQueue(float, float);
        bool doInFromQueue(int64_t);
        void doPrepare();
        void doReset();
        void setGamma(float);
        void setk1(float);
        void setk2(float);
        void setLambda(float);
        indk::Position* getPos() const;
        float getk1() const;
        float getk2() const;
        float getLambda() const;
        int64_t getTl() const;
        float getGamma() const;
        float getdGamma() const;
        int getNeurotransmitterType() const;
        int64_t getQSize();
        ~Synapse() = default;
    };

    class Neuron::Receptor {
    private:
        std::vector<indk::Position*> CP, CPf;
        //indk::Position *RPos, *RPos0, *RPosf;
        std::vector<indk::Position*> ReferencePos;
        indk::Position* DefaultPos;
        indk::Position* PhantomPos;
        float k3;
        float Rs;
        bool Locked;
        float L, Lf;
        float Fi, dFi;
        uint64_t Scope;
    public:
        Receptor();
        Receptor(const indk::Neuron::Receptor&);
        Receptor(indk::Position*, float);
        bool doCheckActive() const;
        void doLock();
        void doUnlock();
        void doCreateNewScope();
        void doChangeScope(uint64_t);
        void doReset();
        void doPrepare();
        void doSavePos();
        void doUpdateSensitivityValue();
        void doUpdatePos(indk::Position*);
        void setPos(indk::Position*);
        void setRs(float);
        void setk3(float);
        void setFi(float);
        std::vector<indk::Position*> getCP() const;
        std::vector<indk::Position*> getCPf() const;
        indk::Position* getPos() const;
        indk::Position* getPos0() const;
        indk::Position* getPosf() const;
        std::vector<indk::Position*> getReferencePosScopes();
        float getRs() const;
        float getk3() const;
        float getFi();
        float getdFi();
        float getSensitivityValue() const;
        bool isLocked() const;
        float getL() const;
        float getLf() const;
        ~Receptor() = default;
    };
}

#endif //INTERFERENCE_NEURON_H
