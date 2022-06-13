#pragma once

#include <string>

#include <core/Cell.h>
#include <core/Direction.h>
#include <core/FlightPlan.h>

class Player;

struct Fleet {
    std::string id;
    Cell *cell;
    Player *player;
    double kore;
    int ships;
    Direction direction;
    FlightPlan flightPlan;

    [[nodiscard]] double getCollectionRate() const;

    void remove() const;
};
