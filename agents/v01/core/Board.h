#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <core/CellMap.h>
#include <core/Configuration.h>
#include <core/Fleet.h>
#include <core/Player.h>
#include <core/Shipyard.h>

class Board {
    int _idCounter;

public:
    static int COPY_CALLS;
    static int NEXT_CALLS;

    Configuration config;

    int step;
    CellMap cells;
    std::vector<std::unique_ptr<Player>> players;

    int meIndex;
    double remainingOverageTime;

    std::unordered_map<std::string, Shipyard *> shipyardsById;
    std::unordered_map<std::string, Fleet *> fleetsById;

    explicit Board(const Configuration &config);

    [[nodiscard]] Player &me();
    [[nodiscard]] const Player &me() const;

    [[nodiscard]] Player &opponent();
    [[nodiscard]] const Player &opponent() const;

    [[nodiscard]] Board copy() const;

    void next();

private:
    void turnResolutionSpawningAndLaunching();
    void turnResolutionFleetsUpdate();
    void turnResolutionAlliedFleetsCoalesce();
    void turnResolutionFleetCollisions();
    void turnResolutionShipyardCollision();
    void turnResolutionFleetToFleetDamage();
    void turnResolutionKoreMining();
    void turnResolutionKoreRegeneration();
    void turnResolutionEndTurn();

    std::string turnResolutionGenerateId();
};
