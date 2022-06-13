#pragma once

#include <strategy/FlightPlanDatabase.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class ExpandComponent : public StrategyComponent {
public:
    explicit ExpandComponent(FlightPlanDatabase &flightPlanDatabase);

    void run(State &state) override;

private:
    [[nodiscard]] int getRequiredShipyards(const State &state) const;
};
