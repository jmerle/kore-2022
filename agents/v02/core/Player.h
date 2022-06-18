#pragma once

#include <memory>
#include <vector>

#include <core/Action.h>
#include <core/Fleet.h>
#include <core/FlightPlan.h>
#include <core/Shipyard.h>

struct Player {
    int id;
    double kore;
    std::vector<std::unique_ptr<Shipyard>> shipyards;
    std::vector<std::unique_ptr<Fleet>> fleets;

    [[nodiscard]] int getShipCount() const;

    void removeShipyard(const Shipyard *shipyard);
    void removeFleet(const Fleet *fleet);
};
