/////////////////////////////////////////////////////////////////////////////
// Name:        indk/position.h
// Purpose:     Multidimensional position class header
// Author:      Nickolay Babbysh
// Created:     07.05.2019
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_POSITION_H
#define INTERFERENCE_POSITION_H

#include <iostream>
#include <cmath>
#include <vector>

namespace indk {
    /// Object position class. Provides the ability to store Cartesian
    /// coordinates of object positions in n-dimensional space,
    /// and also allows you to perform basic arithmetic operations on them:
    /// add, subtract, multiply, and divide.
    class Position {
    private:
        unsigned int Xm;
        unsigned int DimensionsCount;
        float *X;
    public:
        Position();
        Position(const indk::Position&);
        Position(unsigned int, unsigned int);
        Position(unsigned int, std::vector<float>);
        void doAdd(const indk::Position*);
        void doSubtract(const indk::Position*);
        void doDivide(float);
        void doMultiply(float);
        void doZeroPosition();
        void setPosition(const indk::Position&);
        void setPosition(const indk::Position*);
        void setPosition(std::vector<float>);
        void setDimensionsCount(unsigned int);
        void setXm(unsigned int);
        unsigned int getDimensionsCount() const;
        unsigned int getXm() const;
        float getPositionValue(unsigned int) const;
        float getDistanceFrom(const indk::Position*);
        indk::Position& operator= (const indk::Position&);

        static float getDistance(const indk::Position&, const indk::Position&);
        static float getDistance(const indk::Position*, const indk::Position*);

        static indk::Position* getSum(const indk::Position*, const indk::Position*);
        static indk::Position* getDiff(const indk::Position*, const indk::Position*);
        static indk::Position* getQuotient(const indk::Position*, float);
        static indk::Position* getProduct(const indk::Position*, float);
        ~Position();
    };

    indk::Position operator+(const indk::Position&, const indk::Position&);
    indk::Position operator-(const indk::Position&, const indk::Position&);
    indk::Position operator/(const indk::Position&, float);
    indk::Position operator*(const indk::Position&, float);
}

#endif //INTERFERENCE_POSITION_H
