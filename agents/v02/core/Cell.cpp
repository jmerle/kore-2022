#include <algorithm>
#include <cmath>

#include <core/Cell.h>
#include <core/Fleet.h>
#include <core/Shipyard.h>

int Cell::distanceTo(const Cell &other) const {
    return std::abs(other.x - x) + std::abs(other.y - y);
}

void Cell::removeFleet(const Fleet *fleet) {
    fleets.erase(std::remove(fleets.begin(), fleets.end(), fleet), fleets.end());
}
