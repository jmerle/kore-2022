#include <algorithm>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <core/Action.h>
#include <core/Board.h>
#include <strategy/components/AttackFleetComponent.h>

AttackFleetComponent::AttackFleetComponent(FlightPlanDatabase &flightPlanDatabase)
        : StrategyComponent(flightPlanDatabase) {}

void AttackFleetComponent::run(State &state) {
    std::unordered_set<std::string> attackedFleets;

    std::vector<std::pair<int, int>> offsets{
            {1,  0},
            {-1, 0},
            {0,  1},
            {0,  -1}
    };

    Board futureBoard = state.board.copy();
    for (int i = 0; i < 30; i++) {
        futureBoard.next();

        for (const auto &cell : futureBoard.cells) {
            if (cell.shipyard != nullptr || !cell.fleets.empty()) {
                continue;
            }

            std::vector<std::string> fleets;
            int maxAttackSize = std::numeric_limits<int>::max();

            for (const auto &[dx, dy] : offsets) {
                const auto &neighborCell = futureBoard.cells.at(cell.x + dx, cell.y + dy);
                if (!neighborCell.fleets.empty()) {
                    const auto &fleet = neighborCell.fleets[0];
                    if (fleet->player->id != futureBoard.opponent().id) {
                        continue;
                    }

                    if (attackedFleets.find(fleet->id) == attackedFleets.end()) {
                        fleets.push_back(fleet->id);
                        maxAttackSize = std::min(maxAttackSize, fleet->ships);
                    }
                }
            }

            if (fleets.size() < 2) {
                continue;
            }

            for (const auto &shipyard : state.board.me().shipyards) {
                if (shipyard->action.has_value() || state.availableShips[shipyard->id] == 0) {
                    continue;
                }

                int attackSize = std::min(maxAttackSize, state.availableShips[shipyard->id]);

                auto plans = _flightPlanDatabase.getTargetPlans(*shipyard->cell, cell, attackSize, i + 1);
                if (plans.empty()) {
                    continue;
                }

                bool foundPlan = false;
                for (int j = 0; j < 10 && j < plans.size(); j++) {
                    Board testBoard = state.board.copy();
                    testBoard.shipyardsById[shipyard->id]->action = Action::launch(attackSize, plans[j]);

                    for (int k = 0; k <= i; k++) {
                        testBoard.next();
                    }

                    foundPlan = true;
                    for (const auto &fleet : fleets) {
                        if (testBoard.fleetsById.find(fleet) != testBoard.fleetsById.end()
                            && testBoard.fleetsById[fleet]->ships >= futureBoard.fleetsById[fleet]->ships) {
                            foundPlan = false;
                            break;
                        }
                    }

                    if (!foundPlan) {
                        continue;
                    }

                    shipyard->action = Action::launch(attackSize, plans[j]);

                    for (const auto &fleet : fleets) {
                        attackedFleets.insert(fleet);
                    }

                    break;
                }

                if (foundPlan) {
                    break;
                }
            }
        }
    }
}
