#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <core/Action.h>
#include <core/Board.h>
#include <strategy/components/DefendComponent.h>

DefendComponent::DefendComponent(FlightPlanDatabase &flightPlanDatabase)
        : StrategyComponent(flightPlanDatabase) {}

void DefendComponent::run(State &state) {
    std::unordered_map<std::string, std::vector<std::pair<int, int>>> requiredDefenseByShipyards;

    for (const auto &shipyard : state.board.me().shipyards) {
        std::vector<std::pair<int, int>> requiredDefenseBySteps;

        int currentRequiredDefense = 0;
        int previousShips = shipyard->ships;
        bool previousIsMine = true;

        Board futureBoard = state.board.copy();
        futureBoard.opponent().kore = 1e9;

        for (int i = 0; i < 30; i++) {
            futureBoard.next();

            const auto &currentShipyard = futureBoard.cells.at(*shipyard->cell).shipyard;
            if (currentShipyard == nullptr) {
                continue;
            }

            int currentShips = currentShipyard->ships;
            bool currentIsMine = currentShipyard->player->id == futureBoard.me().id;

            if (currentIsMine && currentIsMine == previousIsMine) {
                if (currentShips < previousShips) {
                    currentRequiredDefense += previousShips - currentShips;
                } else if (currentShips > previousShips) {
                    currentRequiredDefense -= currentShips - previousShips;
                }
            } else if (!currentIsMine && currentIsMine == previousIsMine) {
                if (currentShips < previousShips) {
                    currentRequiredDefense -= previousShips - currentShips;
                } else if (currentShips > previousShips) {
                    currentRequiredDefense += currentShips - previousShips;
                }
            } else if (currentIsMine && currentIsMine != previousIsMine) {
                currentRequiredDefense -= previousShips + currentShips;
            } else if (!currentIsMine && currentIsMine != previousIsMine) {
                currentRequiredDefense += previousShips + currentShips;
            }

            if (!currentIsMine) {
                currentShipyard->action = Action::spawn(currentShipyard->getSpawnMaximum());
            }

            if (currentRequiredDefense > 0) {
                requiredDefenseBySteps.emplace_back(i + 1, currentRequiredDefense);
            }

            previousShips = currentShips;
            previousIsMine = currentIsMine;
        }

        if (requiredDefenseBySteps.empty()) {
            continue;
        }

        int maxRequiredDefense = 0;
        for (auto it = requiredDefenseBySteps.begin(); it != requiredDefenseBySteps.end();) {
            maxRequiredDefense = std::max(maxRequiredDefense, it->second);

            it->second -= shipyard->ships;
            if (it->second <= 0) {
                it = requiredDefenseBySteps.erase(it);
            } else {
                it++;
            }
        }

        state.availableShips[shipyard->id] = shipyard->ships - std::min(shipyard->ships, maxRequiredDefense);

        if (requiredDefenseBySteps.empty()) {
            continue;
        }

        spawnMax(state, *shipyard, false);
        int spawning = shipyard->action.has_value() ? shipyard->action->ships : 0;

        for (auto it = requiredDefenseBySteps.begin(); it != requiredDefenseBySteps.end();) {
            it->second -= spawning;
            if (it->second <= 0) {
                it = requiredDefenseBySteps.erase(it);
            } else {
                it++;
            }
        }

        if (requiredDefenseBySteps.empty()) {
            continue;
        }

        requiredDefenseByShipyards[shipyard->id] = std::move(requiredDefenseBySteps);
    }

    int minFleetSize = 5;

    for (const auto &shipyard : state.board.me().shipyards) {
        if (shipyard->action.has_value() || state.availableShips[shipyard->id] < minFleetSize) {
            continue;
        }

        std::vector<Shipyard *> nearbyShipyards;
        for (const auto &otherShipyard : state.board.me().shipyards) {
            if (shipyard->id != otherShipyard->id
                && requiredDefenseByShipyards.find(otherShipyard->id) != requiredDefenseByShipyards.end()) {
                nearbyShipyards.push_back(otherShipyard.get());
            }
        }

        if (nearbyShipyards.empty()) {
            continue;
        }

        std::sort(nearbyShipyards.begin(), nearbyShipyards.end(), [&](const Shipyard *a, const Shipyard *b) {
            return shipyard->cell->distanceTo(*a->cell) < shipyard->cell->distanceTo(*b->cell);
        });

        for (const auto &otherShipyard : nearbyShipyards) {
            int defendingSteps = 0;
            int defendingSize = 0;

            auto &requiredDefenseBySteps = requiredDefenseByShipyards[otherShipyard->id];

            for (int i = 0; i < requiredDefenseBySteps.size(); i++) {
                const auto &[steps, requiredDefense] = requiredDefenseBySteps[i];

                int maxRequiredDefense = 0;
                for (int j = i; j < requiredDefenseBySteps.size(); j++) {
                    maxRequiredDefense = std::max(maxRequiredDefense, requiredDefenseBySteps[i].second);
                }

                int fleetSize = std::max(minFleetSize,
                                         std::min(state.availableShips[shipyard->id], maxRequiredDefense));

                for (int j = 1; j <= steps; j++) {
                    const auto &plans = _flightPlanDatabase.getTargetPlans(*shipyard->cell,
                                                                           *otherShipyard->cell,
                                                                           fleetSize,
                                                                           j);
                    if (!plans.empty()) {
                        shipyard->action = Action::launch(fleetSize, plans[0]);
                        defendingSteps = j;
                        defendingSize = fleetSize;
                        break;
                    }
                }

                if (defendingSteps > 0) {
                    break;
                }
            }

            if (defendingSteps == 0) {
                continue;
            }

            for (auto it = requiredDefenseBySteps.begin(); it != requiredDefenseBySteps.end();) {
                if (it->first < defendingSteps) {
                    it++;
                    continue;
                }

                it->second -= defendingSize;
                if (it->second <= 0) {
                    it = requiredDefenseBySteps.erase(it);
                } else {
                    it++;
                }
            }

            if (requiredDefenseBySteps.empty()) {
                requiredDefenseByShipyards.erase(requiredDefenseByShipyards.find(otherShipyard->id));
            }

            break;
        }
    }
}
