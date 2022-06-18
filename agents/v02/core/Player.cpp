#include <algorithm>

#include <core/Fleet.h>
#include <core/Player.h>
#include <core/Shipyard.h>

int Player::getShipCount() const {
    int ships = 0;

    for (const auto &shipyard : shipyards) {
        ships += shipyard->ships;
    }

    for (const auto &fleet : fleets) {
        ships += fleet->ships;
    }

    return ships;
}

void Player::removeShipyard(const Shipyard *shipyard) {
    shipyards.erase(std::remove_if(shipyards.begin(), shipyards.end(),
                                   [shipyard](const std::unique_ptr<Shipyard> &other) {
                                       return shipyard == other.get();
                                   }),
                    shipyards.end());
}

void Player::removeFleet(const Fleet *fleet) {
    fleets.erase(std::remove_if(fleets.begin(), fleets.end(),
                                [fleet](const std::unique_ptr<Fleet> &other) {
                                    return fleet == other.get();
                                }),
                 fleets.end());
}
