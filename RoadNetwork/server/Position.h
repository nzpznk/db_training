#ifndef __POSITION_H__
#define __POSITION_H__

#include <iostream>

struct Position {
    Position();
    Position(int id, double longitude, double latitude);
    int id;
    double longitude;
    double latitude;
};

std::istream& operator>> (std::istream& is, Position& p);
std::ostream& operator<< (std::ostream& os, Position& p);

#endif
