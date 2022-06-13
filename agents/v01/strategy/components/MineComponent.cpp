#include <algorithm>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

#include <core/Action.h>
#include <strategy/components/MineComponent.h>

MineComponent::MineComponent(FlightPlanDatabase &flightPlanDatabase)
        : StrategyComponent(flightPlanDatabase),
          _randomGenerator(std::random_device{}()) {}

void MineComponent::run(State &state) {
    bool force = shouldForceMining(state);
    int minFleetSize = getMinFleetSize(state.board);
    int maxFleetSize = getMaxFleetSize(state.board);
    int maxSteps = 30;

    std::unordered_map<std::string, std::vector<Action>> possibleActions;

    for (const auto &shipyard : state.board.me().shipyards) {
        if (shipyard->action.has_value() || state.availableShips[shipyard->id] == 0) {
            continue;
        }

        int fleetSize = state.availableShips[shipyard->id];
        if (!force && fleetSize < minFleetSize) {
            continue;
        }

        if (fleetSize > maxFleetSize) {
            fleetSize = maxFleetSize;
        }

        const auto &plans = _flightPlanDatabase.getTargetPlans(*shipyard->cell, *shipyard->cell, fleetSize);
        if (plans.empty()) {
            continue;
        }

        std::vector<Action> actions;
        actions.reserve(plans.size());

        for (int i = 0; i <= maxSteps; i++) {
            const auto &plansForSteps = _flightPlanDatabase.getTargetPlans(*shipyard->cell,
                                                                           *shipyard->cell,
                                                                           fleetSize,
                                                                           i);
            for (const auto &plan : plansForSteps) {
                actions.push_back(Action::launch(fleetSize, plan));
            }
        }

        if (!actions.empty()) {
            possibleActions[shipyard->id] = std::move(actions);
        }
    }

    if (possibleActions.empty()) {
        return;
    }

    std::unordered_map<std::string, int> indices;
    for (auto &[shipyardId, actions] : possibleActions) {
        std::shuffle(actions.begin(), actions.end(), _randomGenerator);
        indices[shipyardId] = 0;
    }

    std::unordered_map<std::string, Action> bestActions;
    double bestScore = std::numeric_limits<double>::lowest();

    double maxMs = state.board.config.actTimeout * 1000 - 250;
    while (state.timer.millisecondsSinceStart() < maxMs) {
        Board currentBoard = state.board.copy();
        std::unordered_map<std::string, Action> currentActions;

        for (auto &[shipyardId, actions] : possibleActions) {
            if (indices[shipyardId] >= actions.size()) {
                std::shuffle(actions.begin(), actions.end(), _randomGenerator);
                indices[shipyardId] = 0;
            }

            const auto &currentAction = actions[indices[shipyardId]];
            currentBoard.shipyardsById[shipyardId]->action = currentAction;
            currentActions.insert({shipyardId, currentAction});

            indices[shipyardId]++;
        }

        std::unordered_set<std::string> existingFleets;
        for (const auto &fleet : state.board.me().fleets) {
            existingFleets.insert(fleet->id);
        }

        std::unordered_map<std::string, double> mineFleetsCargo;
        std::unordered_map<std::string, bool> mineFleetsCloseToHome;

        double currentScore = 0.0;
        bool goodPlan = true;

        for (int i = 0; i < maxSteps; i++) {
            currentBoard.next();

            if (i == 0) {
                for (const auto &fleet : currentBoard.me().fleets) {
                    if (existingFleets.find(fleet->id) == existingFleets.end()) {
                        mineFleetsCargo[fleet->id] = fleet->kore;
                        mineFleetsCloseToHome[fleet->id] = true;
                    }
                }
            } else {
                std::unordered_set<std::string> fleetsSeen;
                for (const auto &fleet : currentBoard.me().fleets) {
                    if (mineFleetsCargo.find(fleet->id) != mineFleetsCargo.end()) {
                        fleetsSeen.insert(fleet->id);
                        mineFleetsCargo[fleet->id] = fleet->kore;

                        mineFleetsCloseToHome[fleet->id] = false;
                        for (const auto &shipyard : currentBoard.me().shipyards) {
                            if (fleet->cell->distanceTo(*shipyard->cell) == 1) {
                                mineFleetsCloseToHome[fleet->id] = true;
                                break;
                            }
                        }
                    }
                }

                auto it = mineFleetsCargo.begin();
                while (it != mineFleetsCargo.end()) {
                    if (fleetsSeen.find(it->first) == fleetsSeen.end()) {
                        if (!mineFleetsCloseToHome[it->first]) {
                            goodPlan = false;
                            break;
                        }

                        currentScore += it->second * ((double) maxSteps / (double) (i + 1));
                        it = mineFleetsCargo.erase(it);
                    } else {
                        it++;
                    }
                }
            }
        }

        if (!goodPlan) {
            continue;
        }

        for (const auto &[fleetId, undeliveredCargo] : mineFleetsCargo) {
            currentScore -= undeliveredCargo;
        }

        if (currentScore > bestScore) {
            bestActions = currentActions;
            bestScore = currentScore;
        }
    }

    for (const auto &[shipyardId, action] : bestActions) {
        state.board.shipyardsById[shipyardId]->action = action;
    }
}

bool MineComponent::shouldForceMining(const State &state) const {
    if (state.board.step < 50
        || !state.board.me().fleets.empty()
        || (state.board.me().kore >= state.board.config.spawnCost && state.savingForEnd)) {
        return false;
    }

    for (const auto &shipyard : state.board.me().shipyards) {
        if (!shipyard->action.has_value()) {
            continue;
        }

        if (shipyard->action->type == ActionType::SPAWN && shipyard->action->ships > 0) {
            return false;
        }
    }

    return true;
}

int MineComponent::getMinFleetSize(const Board &board) const {
    int minSize = std::numeric_limits<int>::max();

    for (const auto &fleet : board.opponent().fleets) {
        minSize = std::min(minSize, fleet->ships);
    }

    if (minSize == std::numeric_limits<int>::max()) {
        return 0;
    }

    return std::min(8, minSize);
}

int MineComponent::getMaxFleetSize(const Board &board) const {
    int maxSize = 0;
    for (const auto &fleet : board.opponent().fleets) {
        maxSize = std::max(maxSize, fleet->ships);
    }

    if (maxSize == 0) {
        return std::numeric_limits<int>::max();
    }

    return std::min(22, maxSize + 1);
}
