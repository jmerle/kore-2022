#include <cmath>
#include <vector>

#include <core/Fleet.h>
#include <core/FlightPlan.h>
#include <strategy/components/MimicComponent.h>

void MimicComponent::run(State &state) {
    Board currentCopy = state.board.copy();

    if (_previousBoard) {
        const auto &previousActions = getPreviousActions(*_previousBoard, state.board);
        double koreLeft = state.board.me().kore;

        for (const auto &shipyard : _previousBoard->opponent().shipyards) {
            auto it = previousActions.find(shipyard->id);
            if (it == previousActions.end()) {
                continue;
            }

            const auto &action = it->second;

            int middle = (state.board.config.size - 1) / 2;
            const auto &myCell = state.board.cells.at(middle + (middle - shipyard->cell->x),
                                                      middle + (middle - shipyard->cell->y));

            if (myCell.shipyard == nullptr || myCell.shipyard->player->id != state.board.me().id) {
                continue;
            }

            if (action.type == ActionType::SPAWN) {
                if (action.ships > myCell.shipyard->getSpawnMaximum()) {
                    continue;
                }

                int requiredKore = action.ships * state.board.config.spawnCost;
                if (requiredKore > koreLeft) {
                    continue;
                }

                myCell.shipyard->action = Action::spawn(action.ships);
                koreLeft -= requiredKore;
            } else {
                if (myCell.shipyard->ships < action.ships) {
                    continue;
                }

                FlightPlan invertedFlightPlan;
                for (const auto &part : action.flightPlan) {
                    switch (part.type) {
                        case FlightPlanPartType::TURN:
                            switch (part.direction) {
                                case Direction::NORTH:
                                    invertedFlightPlan.push_back(FlightPlanPart::turn(Direction::SOUTH));
                                    break;
                                case Direction::EAST:
                                    invertedFlightPlan.push_back(FlightPlanPart::turn(Direction::WEST));
                                    break;
                                case Direction::SOUTH:
                                    invertedFlightPlan.push_back(FlightPlanPart::turn(Direction::NORTH));
                                    break;
                                case Direction::WEST:
                                    invertedFlightPlan.push_back(FlightPlanPart::turn(Direction::EAST));
                                    break;
                            }
                            break;
                        default:
                            invertedFlightPlan.push_back(part);
                    }
                }

                if (invertedFlightPlan.toString().size() > std::floor(2.0 * std::log(myCell.shipyard->ships) + 1)) {
                    continue;
                }

                myCell.shipyard->action = Action::launch(action.ships, invertedFlightPlan);
            }
        }
    }

    // _previousBoard gets messed up when using std::make_unique, need to use std::unique_ptr instead
    _previousBoard = std::unique_ptr<Board>(new Board(currentCopy.copy()));
}

std::unordered_map<std::string, Action> MimicComponent::getPreviousActions(const Board &previousBoard,
                                                                           const Board &currentBoard) const {
    Board previousNext = previousBoard.copy();
    previousNext.next();

    std::vector<Fleet *> newFleets;
    for (const auto &[fleetId, fleet] : currentBoard.fleetsById) {
        const auto &previousNextCell = previousNext.cells.at(*fleet->cell);
        if (previousNextCell.fleets.empty()) {
            newFleets.push_back(fleet);
        }
    }

    std::unordered_map<std::string, Action> previousActions;
    for (const auto &[shipyardId, shipyard] : previousBoard.shipyardsById) {
        for (const auto &newFleet : newFleets) {
            auto direction = directionBetween(*shipyard->cell, *newFleet->cell);
            if (shipyard->cell->distanceTo(*newFleet->cell) == 1 && direction == newFleet->direction) {
                auto originalFlightPlan = newFleet->flightPlan;
                originalFlightPlan.push_front(FlightPlanPart::turn(direction));

                previousActions.insert({shipyardId, Action::launch(newFleet->ships, originalFlightPlan)});
                break;
            }
        }

        if (previousActions.find(shipyardId) != previousActions.end()) {
            continue;
        }

        if (previousNext.shipyardsById.find(shipyardId) != previousNext.shipyardsById.end()
            && currentBoard.shipyardsById.find(shipyardId) != currentBoard.shipyardsById.end()) {
            int newShips =
                    currentBoard.shipyardsById.at(shipyardId)->ships - previousNext.shipyardsById[shipyardId]->ships;
            if (newShips > 0) {
                previousActions.insert({shipyardId, Action::spawn(newShips)});
            }
        }
    }

    return previousActions;
}

Direction MimicComponent::directionBetween(const Cell &from, const Cell &to) const {
    if (from.x < to.x) {
        return Direction::EAST;
    } else if (from.x > to.x) {
        return Direction::WEST;
    } else if (from.y < to.y) {
        return Direction::NORTH;
    } else {
        return Direction::SOUTH;
    }
}
