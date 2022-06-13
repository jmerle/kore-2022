#include <algorithm>
#include <cmath>

#include <core/Action.h>
#include <strategy/components/SpawnGreedyComponent.h>

SpawnGreedyComponent::SpawnGreedyComponent(FlightPlanDatabase &flightPlanDatabase)
        : StrategyComponent(flightPlanDatabase) {}

void SpawnGreedyComponent::run(State &state) {
    if (state.savingForEnd) {
        return;
    }

    int myShips = state.board.me().getShipCount();
    int opponentShips = state.board.opponent().getShipCount();

    int requiredShips = std::max(100, 3 * opponentShips);
    if (myShips >= requiredShips) {
        return;
    }

    int canSpawn = (int) std::floor(state.koreLeft / state.board.config.spawnCost);
    if (canSpawn == 0) {
        return;
    }

    int shipsThreshold = ((double) myShips / 5.0 / (double) state.board.me().shipyards.size());

    for (const auto &shipyard : state.board.me().shipyards) {
        if (shipyard->action.has_value() || shipyard->ships > shipsThreshold) {
            continue;
        }

        shipyard->action = Action::spawn(std::min(canSpawn, shipyard->getSpawnMaximum()));

        myShips += shipyard->action->ships;
        if (myShips >= requiredShips) {
            return;
        }
    }
}
