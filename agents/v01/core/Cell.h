#pragma once

#include <vector>

class Shipyard;

class Fleet;

struct Cell {
    int x;
    int y;

    double kore;

    Shipyard *shipyard;
    std::vector<Fleet *> fleets;

    [[nodiscard]] int distanceTo(const Cell &other) const;

    void removeFleet(const Fleet *Fleet);
};
