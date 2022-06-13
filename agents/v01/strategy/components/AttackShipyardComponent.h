#pragma once

#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class AttackShipyardComponent : public StrategyComponent {
public:
    explicit AttackShipyardComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;
};
