#pragma once

#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class AttackFleetComponent : public StrategyComponent {
public:
    explicit AttackFleetComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;
};
