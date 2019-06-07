#include "Position.h"

Position::Position() {}

Position::Position(int id, double longitude, double latitude):id(id), longitude(longitude), latitude(latitude) {}

std::istream& operator>> (std::istream& is, Position& p) {
    is >> p.id >> p.longitude >> p.latitude;
    return is;
}

std::ostream& operator<< (std::ostream& os, Position& p) {
    os << "(" << p.id << "," << p.longitude << "," << p.latitude << ")";
}
