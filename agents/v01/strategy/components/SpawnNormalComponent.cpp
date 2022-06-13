#include <strategy/components/SpawnNormalComponent.h>

SpawnNormalComponent::SpawnNormalComponent(FlightPlanDatabase &flightPlanDatabase)
        : StrategyComponent(flightPlanDatabase) {}

void SpawnNormalComponent::run(State &state) {
    if (state.savingForEnd) {
        return;
    }

    for (const auto &shipyard : state.board.me().shipyards) {
        if (shipyard->action.has_value()) {
            continue;
        }

        spawnMax(state, *shipyard, true);
    }
}
