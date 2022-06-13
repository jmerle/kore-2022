#pragma once

#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class SpawnNormalComponent : public StrategyComponent {
public:
    explicit SpawnNormalComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;
};
