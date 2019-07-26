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
#include <iostream>
#include "position.h"
#include "computer.hpp"

namespace inn {
    typedef unsigned int TopologyID;

    class Neuron {
    private:
        class Entry;
        class Synaps;
        class Receptor;
        std::vector<inn::Neuron::Entry*> Entries;
        std::vector<inn::Neuron::Receptor*> Receptors;
        std::atomic<unsigned long long> t;
        unsigned int Xm, DimensionsCount;
        float P;
        double Y;
        bool Multithreading;
        std::vector<double> OutputSignalQ;
        inn::Position *dRPos, *nRPos;
        inn::Computer<inn::Neuron::Receptor*, inn::Neuron> *ReceptorPositionComputer;
        bool doPrepareEntriesData(unsigned long long);
        void doComputeNewPosition(inn::Neuron::Receptor*);
        std::vector<double> doCompareCheckpoints();
    public:
        class System;
        Neuron();
        Neuron(const inn::Neuron&);
        Neuron(unsigned int, unsigned int);
        void doEnableMultithreading();
        void doCreateNewEntries(unsigned int);
        void doCreateNewSynaps(unsigned int, std::vector<double>, unsigned int, unsigned int);
        void doCreateNewReceptor(std::vector<double>);
        void doCreateNewReceptorCluster(double, double, double, inn::TopologyID);
        void doSignalsSend();
        void doSignalSendEntry(unsigned long long, double);
        double doSignalReceive();
        double doSignalReceive(unsigned long long);
        bool doCheckOutputSignalQ(unsigned long long);
        void doPrepare();
        void doFinalize();
        void doReinit();
        void doCreateCheckpoint();
        double doComparePattern();
        void setk1(double);
        void setk2(double);
        void setk3(double);
        bool isMultithreadingEnabled() const;
        inn::Neuron::Entry* getEntry(unsigned long long) const;
        inn::Neuron::Receptor* getReceptor(unsigned long long) const;
        unsigned long long getEntriesCount() const;
        unsigned int getSynapsesCount() const;
        unsigned long long getReceptorsCount() const;
        unsigned long long getTime() const;
        unsigned int getXm() const;
        unsigned int getDimensionsCount() const;
        ~Neuron();
    };

    class Neuron::Entry {
    private:
        std::vector<inn::Neuron::Synaps*> Synapses;
        std::vector<double> Signal;
    public:
        Entry() = default;
        Entry(const inn::Neuron::Entry&);
        void doAddSynaps(inn::Position*, unsigned int, unsigned int, int);
        void doIn(double, unsigned long long);
        void doSendToQueue(double, unsigned long long);
        bool doInFromQueue(unsigned long long);
        void doPrepare();
        void doFinalize();
        void doClearSignal();
        void setNeurotransmitterType(int);
        void setk1(double);
        void setk2(double);
        inn::Neuron::Synaps* getSynaps(unsigned long long) const;
        unsigned long long getSynapsesCount() const;
        ~Entry();
    };

    class Neuron::Synaps {
    private:
        inn::Position* SPos;
        double ok1, ok2, k1, k2;
        double Lambda;
        unsigned long long Tl;
        int NeurotransmitterType;
        double Gamma, dGamma;
        long long QCounter;
        std::vector<double> GammaQ;
        std::atomic<unsigned long long> QSize;
    public:
        Synaps();
        Synaps(const inn::Neuron::Synaps&);
        Synaps(inn::Position*, double, double, unsigned long long, int);
        void doIn(double);
        void doSendToQueue(double);
        bool doInFromQueue(unsigned long long);
        void doPrepare();
        void doReset();
        void setk1(double);
        void setk2(double);
        void setNeurotransmitterType(int);
        inn::Position* getPos() const;
        double getk1() const;
        double getk2() const;
        double getLambda() const;
        unsigned long long getTl() const;
        int getNeurotransmitterType() const;
        double getGamma() const;
        double getdGamma() const;
        unsigned long long getQSize();
        ~Synaps() = default;
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

    class Neuron::System {
    public:
        System() = default;
        static std::vector<double> doCompareCPFunction(std::vector<inn::Position*>, std::vector<inn::Position*>);
        static double doCompareCPFunctionD(std::vector<inn::Position*>, std::vector<inn::Position*>);
        static double doCompareFunction(inn::Position*, inn::Position*);
        static double getGammaFunctionValue(double, double, double, double, int);
        static std::pair<double, double> getFiFunctionValue(double, double, double, double);
        static double getReceptorInfluenceValue(bool, double, inn::Position*, inn::Position*);
        static double getRcValue(double, double, double, double);
        static void getNewPosition(inn::Position*, inn::Position*, inn::Position*, double, double);
        static unsigned long long getOutputSignalQMaxSizeValue(unsigned int);
        static unsigned long long getGammaQMaxSizeValue(double);
        static double getLambdaValue(unsigned int);
        static double getFiVectorLength(double);
        static double getSynapticSensitivityValue(unsigned int, unsigned int);
        ~System() = default;
    };
}

#endif //INTERFERENCE_NEURON_H
