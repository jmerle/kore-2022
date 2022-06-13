#include <algorithm>
#include <limits>

#include <core/Action.h>
#include <core/Board.h>
#include <strategy/components/AttackShipyardComponent.h>

AttackShipyardComponent::AttackShipyardComponent(FlightPlanDatabase &flightPlanDatabase)
        : StrategyComponent(flightPlanDatabase) {}

void AttackShipyardComponent::run(State &state) {
    Board futureBoard = state.board.copy();
    futureBoard.opponent().kore = 1e9;

    for (int i = 0; i < 50; i++) {
        for (const auto &shipyard : futureBoard.opponent().shipyards) {
            shipyard->action = Action::spawn(shipyard->getSpawnMaximum());
        }

        futureBoard.next();

        for (const auto &opponentShipyard : futureBoard.opponent().shipyards) {
            int requiredShips = opponentShipyard->ships * 1.2;

            int defendingDistance = std::numeric_limits<int>::max();
            for (const auto &otherOpponentShipyard : futureBoard.opponent().shipyards) {
                if (opponentShipyard->id != otherOpponentShipyard->id) {
                    defendingDistance = std::min(defendingDistance,
                                                 opponentShipyard->cell->distanceTo(*otherOpponentShipyard->cell));
                }
            }

            for (const auto &myShipyard : state.board.me().shipyards) {
                if (myShipyard->action.has_value()
                    || myShipyard->getSpawnMaximum() < 5
                    || state.availableShips[myShipyard->id] < requiredShips
                    || myShipyard->cell->distanceTo(*opponentShipyard->cell) > defendingDistance * 1.5) {
                    continue;
                }

                const auto &plans = _flightPlanDatabase.getTargetPlans(*myShipyard->cell,
                                                                       *opponentShipyard->cell,
                                                                       requiredShips,
                                                                       i + 1);

                if (plans.empty()) {
                    continue;
                }

                Board testBoard = state.board.copy();
                testBoard.opponent().kore = 1e9;

                testBoard.shipyardsById[myShipyard->id]->action = Action::launch(requiredShips, plans[0]);

                for (int j = 0; j <= i; j++) {
                    for (const auto &shipyard : testBoard.opponent().shipyards) {
                        shipyard->action = Action::spawn(shipyard->getSpawnMaximum());
                    }

                    testBoard.next();
                }

                const auto &targetCell = testBoard.cells.at(*opponentShipyard->cell);
                if (targetCell.shipyard != nullptr && targetCell.shipyard->player->id == myShipyard->player->id) {
                    myShipyard->action = Action::launch(requiredShips, plans[0]);
                }
            }
        }
    }
}
