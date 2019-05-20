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

#include <iostream>
#include <vector>
#include <cmath>
#include "position.h"

namespace inn {
    typedef unsigned int TopologyID;

    class Neuron {
    private:
        class Entry;
        class Synaps;
        class Receptor;
        class System;
        std::vector<inn::Neuron::Entry*> Entries;
        std::vector<inn::Neuron::Receptor*> Receptors;
        unsigned long long t;
        unsigned int Xm, DimensionsCount;
        float P;
        std::vector<double> doCompareCheckpoints();
    public:
        Neuron();
        Neuron(const inn::Neuron&);
        Neuron(unsigned int, unsigned int);
        void doCreateNewEntry();
        void doCreateNewSynaps(unsigned int, inn::Position, unsigned int);
        void doCreateNewSynaps(unsigned int, inn::Position, unsigned int, unsigned int);
        void doCreateNewReceptor(inn::Position);
        void doCreateNewReceptorCluster(double, double, double, inn::TopologyID);
        float doSignalsSend(std::vector<double>);
        bool doSignalReceive();
        void doFinalize();
        void doCreateCheckpoint();
        double doComparePattern();
        double doComparePattern(bool);
        std::vector<Entry*> getEntries() const;
        std::vector<Receptor*> getReceptors() const;
        unsigned long long getEntriesCount();
        unsigned int getSynapsesCount();
        unsigned long long getReceptorsCount();
        unsigned int getDimensionsCount();
        ~Neuron();
    };

    class Neuron::Entry {
    private:
        std::vector<inn::Neuron::Synaps*> Synapses;
        std::vector<double> Signal;
    public:
        Entry() = default;
        Entry(const inn::Neuron::Entry&);
        void doAddSynaps(inn::Position, unsigned int, int);
        void doIn(double, unsigned long long);
        void doClearSignal();
        inn::Neuron::Synaps* getSynaps(unsigned long long) const;
        unsigned long long getSynapsesCount() const;
        ~Entry() = default;
    };

    class Neuron::Synaps {
    private:
        inn::Position SPos;
        double k1, k2;
        double Lambda;
        unsigned long long Tl;
        int NeurotransmitterType;
        double Gamma, dGamma;
    public:
        Synaps();
        Synaps(const inn::Neuron::Synaps&);
        Synaps(inn::Position, unsigned long long, int);
        void doIn(double);
        void doClearGamma();
        inn::Position getPos() const;
        double getk1() const;
        double getk2() const;
        double getLambda() const;
        unsigned long long getTl() const;
        int getNeurotransmitterType() const;
        double getGamma() const;
        double getdGamma() const;
        ~Synaps() = default;
    };

    class Neuron::Receptor {
    private:
        std::vector<inn::Position> CP, CPf;
        inn::Position RPos, RPos0, RPosf;
        double k3;
        double Rs;
        bool Locked;
        double L, Lf;
        double Fi, dFi;
    public:
        Receptor();
        Receptor(const inn::Neuron::Receptor&);
        Receptor(inn::Position, double);
        bool doCheckActive();
        void doLock();
        void doUnlock();
        void doSavePos();
        void doUpdateSensitivityValue();
        void setPos(inn::Position);
        void setFi(double);
        std::vector<inn::Position> getCP() const;
        std::vector<inn::Position> getCPf() const;
        inn::Position getPos() const;
        inn::Position getPos0() const;
        inn::Position getPosf() const;
        double getk3() const;
        double getSensitivityValue() const;
        bool isLocked() const;
        double getL() const;
        double getLf() const;
        ~Receptor() = default;
    };

    class Neuron::System {
    public:
        System() = default;
        static std::vector<double> doCompareCPFunction(std::vector<inn::Position>, std::vector<inn::Position>);
        static double doCompareCPFunctionD(std::vector<inn::Position>, std::vector<inn::Position>);
        static double doCompareFunction(inn::Position, inn::Position, double, double);
        static double getGammaFunctionValue(double, double, double, double, int);
        static double getFiFunctionValue(inn::Position, inn::Position, double, double);
        static double getRcValue(double, double, double, double);
        static inn::Position getNewPosition(inn::Position, inn::Position, double);
        static double getFiVectorLength(double);
        ~System() = default;
    };
}

#endif //INTERFERENCE_NEURON_H
