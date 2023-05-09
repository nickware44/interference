/////////////////////////////////////////////////////////////////////////////
// Name:        inn/position.h
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

namespace inn {
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
        Position(const inn::Position&);
        Position(unsigned int, unsigned int);
        Position(unsigned int, std::vector<float>);
        void doAdd(const inn::Position*);
        void doSubtract(const inn::Position*);
        void doDivide(float);
        void doMultiply(float);
        void doZeroPosition();
        void setPosition(const inn::Position&);
        void setPosition(const inn::Position*);
        void setPosition(std::vector<float>);
        void setDimensionsCount(unsigned int);
        void setXm(unsigned int);
        unsigned int getDimensionsCount() const;
        unsigned int getXm() const;
        float getPositionValue(unsigned int) const;
        float getDistanceFrom(const inn::Position*);
        inn::Position& operator= (const inn::Position&);

        static float getDistance(const inn::Position&, const inn::Position&);
        static float getDistance(const inn::Position*, const inn::Position*);

        static inn::Position* getSum(const inn::Position*, const inn::Position*);
        static inn::Position* getDiff(const inn::Position*, const inn::Position*);
        static inn::Position* getQuotient(const inn::Position*, float);
        static inn::Position* getProduct(const inn::Position*, float);
        ~Position();
    };

    inn::Position operator+(const inn::Position&, const inn::Position&);
    inn::Position operator-(const inn::Position&, const inn::Position&);
    inn::Position operator/(const inn::Position&, float);
    inn::Position operator*(const inn::Position&, float);
}

#endif //INTERFERENCE_POSITION_H
