#ifndef __CAR_H__
#define __CAR_H__

#include <vector>
#include <iostream>
#include "Position.h"

struct Car {
    Car();
    int cid;
    int cust_num;
    int d1;
    Position p;
    std::vector<Position> dst_list;
    std::vector<int> order;
    int mat[5][5];
};

std::istream& operator>> (std::istream& is, Car& car);
std::ostream& operator<< (std::ostream& os, Car& car);

#endif
