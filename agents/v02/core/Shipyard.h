#pragma once

#include <optional>
#include <string>

#include <core/Action.h>
#include <core/Cell.h>

class Player;

struct Shipyard {
    std::string id;
    Cell *cell;
    Player *player;
    int ships;
    int turnsControlled;
    std::optional<Action> action;

    [[nodiscard]] int getSpawnMaximum() const;

    void remove() const;
};
