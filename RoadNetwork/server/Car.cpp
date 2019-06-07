#include "Car.h"

Car::Car() {} 

std::istream& operator>> (std::istream& is, Car& car) {
    int posi_id;
    double longitude, latitude;
    char comma;
    car.dst_list.clear();
    is >> car.cid >> car.cust_num;
    is >> car.p.longitude >> comma >> car.p.latitude >> comma >> car.p.id;
    for (int i = 0; i < car.cust_num; ++i) {
        is >> longitude >> comma >> latitude >> comma >> posi_id;
        car.dst_list.emplace_back(posi_id, longitude, latitude);
    }
    return is;
}

std::ostream& operator<< (std::ostream& os, Car& car) {
    os << "{ " << car.cid << ", " << car.cust_num << ", " << car.p << ", ";
    os << "[";
    for (auto& p : car.dst_list) {
        os << p;
    }
    os << "]";
    os << " }";
}
