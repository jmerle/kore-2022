#include <core/Player.h>
#include <core/Shipyard.h>

int Shipyard::getSpawnMaximum() const {
    if (turnsControlled < 2) {
        return 1;
    } else if (turnsControlled < 7) {
        return 2;
    } else if (turnsControlled < 17) {
        return 3;
    } else if (turnsControlled < 34) {
        return 4;
    } else if (turnsControlled < 60) {
        return 5;
    } else if (turnsControlled < 97) {
        return 6;
    } else if (turnsControlled < 147) {
        return 7;
    } else if (turnsControlled < 212) {
        return 8;
    } else if (turnsControlled < 294) {
        return 9;
    } else {
        return 10;
    }
}

void Shipyard::remove() const {
    cell->shipyard = nullptr;
    player->removeShipyard(this);
}
