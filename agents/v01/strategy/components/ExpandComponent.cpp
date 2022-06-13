#include <algorithm>
#include <limits>
#include <unordered_set>

#include <core/Board.h>
#include <core/FlightPlan.h>
#include <strategy/components/ExpandComponent.h>

ExpandComponent::ExpandComponent(FlightPlanDatabase &flightPlanDatabase) : StrategyComponent(flightPlanDatabase) {}

void ExpandComponent::run(State &state) {
    int requiredShipyards = getRequiredShipyards(state);
    if (requiredShipyards == 0) {
        return;
    }

    int minDistance = 3;
    int maxDistance = 5;
    int requiredShips = state.board.config.convertCost;

    Board futureBoard = state.board.copy();
    for (int i = 0; i < maxDistance * 2; i++) {
        futureBoard.next();
    }

    std::unordered_set<Cell *> usedCells;

    for (const auto &shipyard : state.board.me().shipyards) {
        if (shipyard->action.has_value() || shipyard->getSpawnMaximum() < 5) {
            continue;
        }

        int fleetSize = state.availableShips[shipyard->id];
        if (fleetSize < requiredShips) {
            continue;
        }

        Cell *bestCell = nullptr;
        double bestScore = std::numeric_limits<double>::lowest();

        for (auto &targetCell : state.board.cells) {
            if (targetCell.shipyard != nullptr
                || futureBoard.cells.at(targetCell).shipyard != nullptr
                || usedCells.find(&targetCell) != usedCells.end()) {
                continue;
            }

            int distance = targetCell.distanceTo(*shipyard->cell);
            if (distance < minDistance || distance > maxDistance) {
                continue;
            }

            double nearbyKore = 0.0;
            int nearbyShipyards = 0;

            int closestFriendly = std::numeric_limits<int>::max();
            int closestOpponent = std::numeric_limits<int>::max();

            for (const auto &otherCell : state.board.cells) {
                int distanceToOther = targetCell.distanceTo(otherCell);
                if (distanceToOther == 0) {
                    continue;
                }

                if (distanceToOther <= 8) {
                    nearbyKore += otherCell.kore;
                }

                if (otherCell.shipyard != nullptr) {
                    if (distanceToOther <= maxDistance) {
                        nearbyShipyards++;
                    }

                    if (otherCell.shipyard->player->id == state.board.me().id) {
                        closestFriendly = std::min(closestFriendly, distanceToOther);
                    } else {
                        closestOpponent = std::min(closestFriendly, distanceToOther);
                    }
                }
            }

            if (closestOpponent < closestFriendly) {
                continue;
            }

            int closestOther = std::min(closestFriendly, closestOpponent);
            if (closestOther < minDistance || closestOther > maxDistance) {
                continue;
            }

            double score = nearbyKore - 1000 * nearbyShipyards - 1000 * closestOther;
            if (score > bestScore) {
                bestCell = &targetCell;
                bestScore = score;
            }
        }

        if (bestCell == nullptr) {
            continue;
        }

        const auto &plans = _flightPlanDatabase.getConvertPlans(*shipyard->cell, *bestCell, fleetSize);
        std::string bestPlan;

        for (int i = 0; i < 10 && i < plans.size(); i++) {
            Board testBoard = state.board.copy();
            testBoard.shipyardsById[shipyard->id]->action = Action::launch(fleetSize, plans[i]);

            for (int j = 0; j < 50; j++) {
                testBoard.next();
            }

            const auto &targetCell = testBoard.cells.at(*bestCell);
            if (targetCell.shipyard != nullptr && targetCell.shipyard->player->id == state.board.me().id) {
                bestPlan = plans[i];
                break;
            }
        }

        if (!bestPlan.empty()) {
            shipyard->action = Action::launch(fleetSize, bestPlan);

            requiredShipyards--;
            if (requiredShipyards == 0) {
                return;
            }

            usedCells.insert(bestCell);
        }
    }
}

int ExpandComponent::getRequiredShipyards(const State &state) const {
    int myCurrentShips = state.board.me().getShipCount();
    if (myCurrentShips < 100) {
        return 0;
    }

    int maxSpawn = 0;
    for (const auto &shipyard : state.board.me().shipyards) {
        maxSpawn += shipyard->getSpawnMaximum();
    }

    int stepsLeft = state.board.config.episodeSteps - state.board.step;
    int scale;
    if (stepsLeft > 100) {
        scale = 15;
    } else if (stepsLeft > 50) {
        scale = 20;
    } else if (stepsLeft > 10) {
        scale = 1000;
    } else {
        scale = 10000;
    }

    if (state.board.me().kore < scale * maxSpawn) {
        return 0;
    }

    Board futureBoard = state.board.copy();
    for (int i = 0; i < 50; i++) {
        futureBoard.next();
    }

    int myCurrentShipyards = state.board.me().shipyards.size();
    int myFutureShipyards = futureBoard.me().shipyards.size();
    int opponentFutureShipyards = futureBoard.opponent().shipyards.size();

    int opponentCurrentShips = state.board.opponent().getShipCount();

    if (myFutureShipyards > opponentFutureShipyards && myCurrentShips < opponentCurrentShips) {
        return 0;
    }

    if (myCurrentShipyards < 10) {
        return myFutureShipyards > myCurrentShipyards ? 0 : 1;
    }

    return std::max(0, 5 - (myFutureShipyards - myCurrentShipyards));
}
