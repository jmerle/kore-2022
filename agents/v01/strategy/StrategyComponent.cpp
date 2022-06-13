#include <algorithm>
#include <cmath>

#include <core/Action.h>
#include <strategy/StrategyComponent.h>

StrategyComponent::StrategyComponent(FlightPlanDatabase &flightPlanDatabase)
        : _flightPlanDatabase(flightPlanDatabase) {}

void StrategyComponent::spawnMax(State &state, Shipyard &shipyard, bool allowZero) const {
    double spawnCost = state.board.config.spawnCost;

    int maxSpawn = std::min((int) std::floor(state.koreLeft / spawnCost), shipyard.getSpawnMaximum());
    if (!allowZero && maxSpawn == 0) {
        return;
    }

    shipyard.action = Action::spawn(maxSpawn);
    state.koreLeft -= maxSpawn * spawnCost;
}
