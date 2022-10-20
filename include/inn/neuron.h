/////////////////////////////////////////////////////////////////////////////
// Name:        inn/neuron.h
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
#include "position.h"

namespace inn {
    typedef unsigned int TopologyID;
    enum class WaveType: int {NOWAVE = -1, WAVETYPE1, WAVETYPE2, WAVETYPE3};
    typedef std::pair<inn::WaveType, double> WaveDefinition;

    class Neuron {
    private:
        class Entry;
        class Synapse;
        class Receptor;
        std::map<std::string, inn::Neuron::Entry*> Entries;
        std::vector<std::string> Links;
        std::vector<inn::Neuron::Receptor*> Receptors;
        std::atomic<int64_t> t;
        int64_t Tlo;
        unsigned int Xm, DimensionsCount;
        double Y;
        int NID;
        bool Learned;
        std::vector<double> OutputSignalQ;
        inn::WaveType WTin, WTout;
        std::vector<double> doCompareCheckpoints();
        double LastWVSum;
        bool Pending;
    public:
        typedef std::tuple<double, double> PatternDefinition;
        Neuron();
        Neuron(const inn::Neuron&);
        Neuron(unsigned int, unsigned int, int64_t, const std::vector<std::string>& InputSignals);
        void doCreateNewSynapse(const std::string&, std::vector<double>, unsigned int);
        void doCreateNewReceptor(std::vector<double>);
        void doCreateNewReceptorCluster(double, double, double, inn::TopologyID);
        bool doSignalSendEntry(const std::string&, double, const std::vector<inn::WaveDefinition>&);
        std::pair<int64_t, double> doSignalReceive();
        double doSignalReceive(int64_t);
        bool doCheckOutputSignalQ(int64_t);
        void doFinalizeInput(double);
        void doPrepare();
        void doFinalize();
        void doReinit();
        void doCreateCheckpoint();
        inn::Neuron::PatternDefinition doComparePattern() const;
        void doLinkOutput(const std::string&);
        void setTime(int64_t);
        void setk1(double);
        void setk2(double);
        void setk3(double);
        void setNID(int);
        std::vector<std::string>& getLinkOutput();
        std::vector<std::string> getEntries();
        inn::Neuron::Entry*  getEntry(int64_t);
        inn::Neuron::Receptor* getReceptor(int64_t) const;
        std::vector<std::string> getWaitingEntries();
        int64_t getEntriesCount() const;
        unsigned int getSynapsesCount() const;
        int64_t getReceptorsCount() const;
        int64_t getTime() const;
        unsigned int getXm() const;
        unsigned int getDimensionsCount() const;
        inn::WaveType getWTin() const;
        inn::WaveType getWTout() const;
        inn::WaveDefinition getWave() const;
        int64_t getTlo() const;
        int getNID() const;
        bool isPending() const;
        ~Neuron();
    };

    class Neuron::Entry {
    private:
        std::vector<inn::Neuron::Synapse*> Synapses;
        double X;
        int64_t t;
        //std::vector<double> Signal;
    public:
        Entry();
        Entry(const inn::Neuron::Entry&);
        bool doCheckState(int64_t) const;
        void doAddSynapse(inn::Position*, unsigned int, unsigned int);
        void doIn(double, int64_t);
        void doProcess();
        void doSendToQueue(double, int64_t, double);
        bool doInFromQueue(int64_t);
        void doPrepare();
        void doFinalize();
        void doClearSignal();
        void setWTs(inn::WaveType);
        void setk1(double);
        void setk2(double);
        inn::Neuron::Synapse* getSynapse(int64_t) const;
        int64_t getSynapsesCount() const;
        ~Entry();
    };

    class Neuron::Synapse {
    private:
        inn::Position* SPos;
        double ok1, ok2, k1, k2;
        double Lambda;
        int64_t Tl;
        inn::WaveType WTs;
        double Gamma, dGamma;
        long long QCounter;
        std::vector<double> GammaQ;
        std::atomic<int64_t> QSize;
    public:
        Synapse();
        Synapse(const inn::Neuron::Synapse&);
        Synapse(inn::Position*, double, double, int64_t);
        void doIn(double);
        void doSendToQueue(double, double);
        bool doInFromQueue(int64_t);
        void doPrepare();
        void doReset();
        void setk1(double);
        void setk2(double);
        void setWTs(inn::WaveType);
        inn::Position* getPos() const;
        double getk1() const;
        double getk2() const;
        double getLambda() const;
        int64_t getTl() const;
        inn::WaveType getWTs() const;
        double getGamma() const;
        double getdGamma() const;
        int64_t getQSize();
        ~Synapse() = default;
    };

    class Neuron::Receptor {
    private:
        std::vector<inn::Position*> CP, CPf;
        inn::Position *RPos, *RPos0, *RPosf;
        double k3;
        double Rs;
        bool Locked;
        double L, Lf;
        double Fi, dFi;
    public:
        Receptor();
        Receptor(const inn::Neuron::Receptor&);
        Receptor(inn::Position*, double);
        bool doCheckActive();
        void doLock();
        void doUnlock();
        void doReset();
        void doPrepare();
        void doSavePos();
        void doUpdateSensitivityValue();
        void setPos(inn::Position*);
        void setk3(double);
        void setFi(double);
        std::vector<inn::Position*> getCP() const;
        std::vector<inn::Position*> getCPf() const;
        inn::Position* getPos() const;
        inn::Position* getPos0() const;
        inn::Position* getPosf() const;
        double getk3() const;
        double getdFi();
        double getSensitivityValue() const;
        bool isLocked() const;
        double getL() const;
        double getLf() const;
        ~Receptor() = default;
    };
}

#endif //INTERFERENCE_NEURON_H
