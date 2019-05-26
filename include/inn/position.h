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
    class Position {
    private:
        unsigned int Xm;
        unsigned int DimensionsCount;
        double *X;
    public:
        Position();
        Position(const inn::Position&);
        Position(unsigned int, unsigned int);
        Position(unsigned int, std::vector<double>);
        void setPosition(const inn::Position&);
        void setPosition(std::vector<double>);
        unsigned int getDimensionsCount() const;
        unsigned int getXm() const;
        double getPositionValue(unsigned int) const;
        inn::Position& operator= (const inn::Position&);
        static const double getDistance(const inn::Position&, const inn::Position&);
        ~Position();
    };

    const inn::Position operator+(const inn::Position&, const inn::Position&);
    const inn::Position operator-(const inn::Position&, const inn::Position&);
    const inn::Position operator/(const inn::Position&, double);
    const inn::Position operator*(const inn::Position&, double);
}

#endif //INTERFERENCE_POSITION_H
