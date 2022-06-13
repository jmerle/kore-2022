#pragma once

#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class SpawnGreedyComponent : public StrategyComponent {
public:
    explicit SpawnGreedyComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;
};
