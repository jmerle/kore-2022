#pragma once

#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class DefendComponent : public StrategyComponent {
public:
    explicit DefendComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;
};
