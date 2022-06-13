#pragma once

#include <core/Shipyard.h>
#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>

class StrategyComponent {
protected:
    FlightPlanDatabase &_flightPlanDatabase;

public:
    explicit StrategyComponent(FlightPlanDatabase &flightPlanDatabase);

    virtual ~StrategyComponent() = default;

    virtual void run(State &state) = 0;

protected:
    void spawnMax(State &state, Shipyard &shipyard, bool allowZero) const;
};
