#include <algorithm>
#include <cmath>

#include <core/Fleet.h>
#include <core/Player.h>

double Fleet::getCollectionRate() const {
    return std::min(std::log(ships) / 20.0, 0.99);
}

void Fleet::remove() const {
    cell->removeFleet(this);
    player->removeFleet(this);
}
