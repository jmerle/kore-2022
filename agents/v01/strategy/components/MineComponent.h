#pragma once

#include <random>

#include <core/Board.h>
#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class MineComponent : public StrategyComponent {
    std::mt19937 _randomGenerator;

public:
    explicit MineComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;

private:
    [[nodiscard]] bool shouldForceMining(const State &state) const;

    [[nodiscard]] int getMinFleetSize(const Board &board) const;
    [[nodiscard]] int getMaxFleetSize(const Board &board) const;
};
