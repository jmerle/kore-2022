#include <algorithm>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <utility>

#include <core/Action.h>
#include <core/Board.h>
#include <core/Shipyard.h>
#include <core/Fleet.h>

int Board::COPY_CALLS = 0;
int Board::NEXT_CALLS = 0;

Board::Board(const Configuration &config)
        : _idCounter(1),
          config(config),
          step(),
          cells(config.size),
          players(),
          meIndex(),
          remainingOverageTime(),
          shipyardsById(),
          fleetsById() {}

Player &Board::me() {
    return *players[meIndex];
}

const Player &Board::me() const {
    return *players[meIndex];
}

Player &Board::opponent() {
    return *players[meIndex == 0 ? 1 : 0];
}

const Player &Board::opponent() const {
    return *players[meIndex == 0 ? 1 : 0];
}

Board Board::copy() const {
    COPY_CALLS++;

    Board newBoard(config);

    newBoard.step = step;
    newBoard.meIndex = meIndex;
    newBoard.remainingOverageTime = remainingOverageTime;

    for (int i = 0, iMax = config.size * config.size; i < iMax; i++) {
        newBoard.cells.at(i).kore = cells.at(i).kore;
    }

    newBoard.players.reserve(players.size());
    for (const auto &player : players) {
        auto newPlayer = std::make_unique<Player>();
        newPlayer->id = player->id;
        newPlayer->kore = player->kore;

        newPlayer->shipyards.reserve(player->shipyards.size());
        for (const auto &shipyard : player->shipyards) {
            auto newShipyard = std::make_unique<Shipyard>();
            newShipyard->id = shipyard->id;
            newShipyard->cell = &newBoard.cells.at(shipyard->cell->x, shipyard->cell->y);
            newShipyard->player = newPlayer.get();
            newShipyard->ships = shipyard->ships;
            newShipyard->turnsControlled = shipyard->turnsControlled;
            newShipyard->action = shipyard->action;

            newBoard.shipyardsById[newShipyard->id] = newShipyard.get();

            newShipyard->cell->shipyard = newShipyard.get();
            newPlayer->shipyards.push_back(std::move(newShipyard));
        }

        newPlayer->fleets.reserve(player->fleets.size());
        for (const auto &fleet : player->fleets) {
            auto newFleet = std::make_unique<Fleet>();
            newFleet->id = fleet->id;
            newFleet->cell = &newBoard.cells.at(fleet->cell->x, fleet->cell->y);
            newFleet->player = newPlayer.get();
            newFleet->kore = fleet->kore;
            newFleet->ships = fleet->ships;
            newFleet->direction = fleet->direction;
            newFleet->flightPlan = FlightPlan(fleet->flightPlan);

            newBoard.fleetsById[newFleet->id] = newFleet.get();

            newFleet->cell->fleets.push_back(newFleet.get());
            newPlayer->fleets.push_back(std::move(newFleet));
        }

        newBoard.players.push_back(std::move(newPlayer));
    }

    return newBoard;
}

void Board::next() {
    NEXT_CALLS++;

    _idCounter = 1;

    turnResolutionSpawningAndLaunching();
    turnResolutionFleetsUpdate();
    turnResolutionAlliedFleetsCoalesce();
    turnResolutionFleetCollisions();
    turnResolutionShipyardCollision();
    turnResolutionFleetToFleetDamage();
    turnResolutionKoreMining();
    turnResolutionKoreRegeneration();
    turnResolutionEndTurn();
}

void Board::turnResolutionSpawningAndLaunching() {
    for (auto &player : players) {
        for (const auto &shipyard : player->shipyards) {
            if (!shipyard->action.has_value()) {
                continue;
            }

            int ships = shipyard->action->ships;
            if (ships == 0) {
                continue;
            }

            if (shipyard->action->type == ActionType::SPAWN) {
                double spawnCost = config.spawnCost * shipyard->action->ships;
                if (spawnCost > player->kore || ships > shipyard->getSpawnMaximum()) {
                    continue;
                }

                shipyard->ships += ships;
                player->kore -= spawnCost;
            } else {
                if (ships > shipyard->ships) {
                    continue;
                }

                std::string flightPlan = shipyard->action->flightPlan.toString();

                int maxFlightPlanLength = std::floor(2 * std::log(ships)) + 1;
                if (flightPlan.size() > maxFlightPlanLength) {
                    flightPlan = flightPlan.substr(0, maxFlightPlanLength);
                }

                shipyard->ships -= ships;

                auto newFleet = std::make_unique<Fleet>();
                newFleet->id = turnResolutionGenerateId();
                newFleet->cell = shipyard->cell;
                newFleet->player = shipyard->player;
                newFleet->kore = 0.0;
                newFleet->ships = ships;
                newFleet->direction = shipyard->action->flightPlan[0].direction;
                newFleet->flightPlan = FlightPlan::parse(flightPlan);

                fleetsById[newFleet->id] = newFleet.get();

                player->fleets.push_back(std::move(newFleet));
            }
        }

        for (const auto &shipyard : player->shipyards) {
            shipyard->action.reset();
            shipyard->turnsControlled++;
        }
    }
}

void Board::turnResolutionFleetsUpdate() {
    for (auto &player : players) {
        auto it = player->fleets.begin();
        while (it != player->fleets.end()) {
            auto *fleet = it->get();

            auto &flightPlan = fleet->flightPlan;

            while (!flightPlan.empty()
                   && flightPlan.front().type == FlightPlanPartType::MOVE
                   && flightPlan.front().steps == 0) {
                flightPlan.pop_front();
            }

            if (!flightPlan.empty()
                && flightPlan.front().type == FlightPlanPartType::CONVERT
                && fleet->ships >= config.convertCost
                && fleet->cell->shipyard == nullptr) {
                player->kore += fleet->kore;
                fleet->cell->kore = 0.0;

                auto newShipyard = std::make_unique<Shipyard>();
                newShipyard->id = turnResolutionGenerateId();
                newShipyard->cell = fleet->cell;
                newShipyard->player = fleet->player;
                newShipyard->ships = fleet->ships - config.convertCost;
                newShipyard->turnsControlled = 0;

                shipyardsById[newShipyard->id] = newShipyard.get();

                newShipyard->cell->shipyard = newShipyard.get();
                player->shipyards.push_back(std::move(newShipyard));

                fleet->cell->removeFleet(fleet);
                it = player->fleets.erase(it);
                continue;
            }

            while (!flightPlan.empty() && flightPlan.front().type == FlightPlanPartType::CONVERT) {
                flightPlan.pop_front();
            }

            if (!flightPlan.empty()) {
                switch (flightPlan.front().type) {
                    case FlightPlanPartType::TURN:
                        fleet->direction = flightPlan.front().direction;
                        flightPlan.pop_front();
                        break;
                    case FlightPlanPartType::MOVE:
                        if (flightPlan.front().steps == 1) {
                            flightPlan.pop_front();
                        } else {
                            flightPlan.front().steps--;
                        }
                        break;
                    case FlightPlanPartType::CONVERT:
                        break;
                }
            }

            Cell *currentCell = fleet->cell;
            Cell *newCell = nullptr;

            switch (fleet->direction) {
                case Direction::NORTH:
                    newCell = &cells.at(currentCell->x, currentCell->y + 1);
                    break;
                case Direction::EAST:
                    newCell = &cells.at(currentCell->x + 1, currentCell->y);
                    break;
                case Direction::SOUTH:
                    newCell = &cells.at(currentCell->x, currentCell->y - 1);
                    break;
                case Direction::WEST:
                    newCell = &cells.at(currentCell->x - 1, currentCell->y);
                    break;
            }

            currentCell->removeFleet(fleet);
            fleet->cell = newCell;
            fleet->cell->fleets.push_back(fleet);

            it++;
        }
    }
}

void Board::turnResolutionAlliedFleetsCoalesce() {
    for (const auto &player : players) {
        for (const auto &cell : cells) {
            if (cell.fleets.size() < 2) {
                continue;
            }

            std::vector<Fleet *> alliedFleets;
            for (const auto &fleet : cell.fleets) {
                if (fleet->player == player.get()) {
                    alliedFleets.push_back(fleet);
                }
            }

            if (alliedFleets.size() < 2) {
                continue;
            }

            const auto &biggestAlly = *std::max_element(alliedFleets.begin(), alliedFleets.end(),
                                                        [](const Fleet *a, const Fleet *b) {
                                                            if (a->ships == b->ships) {
                                                                if (a->kore == b->kore) {
                                                                    return a->direction > b->direction;
                                                                } else {
                                                                    return a->kore < b->kore;
                                                                }
                                                            } else {
                                                                return a->ships < b->ships;
                                                            }
                                                        });

            for (const auto &fleet : alliedFleets) {
                if (fleet == biggestAlly) {
                    continue;
                }

                biggestAlly->kore += fleet->kore;
                biggestAlly->ships += fleet->ships;

                fleetsById.erase(fleetsById.find(fleet->id));
                fleet->remove();
            }
        }
    }
}

void Board::turnResolutionFleetCollisions() {
    for (const auto &cell : cells) {
        if (cell.fleets.size() < 2) {
            continue;
        }

        Fleet *biggestFleet = nullptr;
        bool tied = false;

        for (const auto &fleet : cell.fleets) {
            if (biggestFleet == nullptr) {
                biggestFleet = fleet;
            } else if (fleet->ships > biggestFleet->ships) {
                biggestFleet = fleet;
            } else if (fleet->ships == biggestFleet->ships) {
                tied = true;
                break;
            }
        }

        std::vector<Fleet *> battlingFleets(cell.fleets);
        for (const auto &fleet : battlingFleets) {
            if (!tied && biggestFleet == fleet) {
                continue;
            }

            if (tied) {
                if (fleet->cell->shipyard == nullptr) {
                    fleet->cell->kore += fleet->kore;
                } else {
                    fleet->cell->shipyard->player->kore += fleet->kore;
                }
            } else {
                biggestFleet->kore += fleet->kore;
                biggestFleet->ships -= fleet->ships;
            }

            fleetsById.erase(fleetsById.find(fleet->id));
            fleet->remove();
        }
    }
}

void Board::turnResolutionShipyardCollision() {
    for (const auto &cell : cells) {
        if (cell.shipyard == nullptr || cell.fleets.empty()) {
            continue;
        }

        const auto &shipyard = cell.shipyard;
        const auto &fleet = cell.fleets[0];

        if (shipyard->player == fleet->player) {
            fleet->player->kore += fleet->kore;
            shipyard->ships += fleet->ships;

            fleetsById.erase(fleetsById.find(fleet->id));
            fleet->remove();
        } else {
            if (fleet->ships <= shipyard->ships) {
                shipyard->ships -= fleet->ships;
                shipyard->player->kore += fleet->kore;

                fleetsById.erase(fleetsById.find(fleet->id));
                fleet->remove();
            } else {
                auto newShipyard = std::make_unique<Shipyard>();
                newShipyard->id = turnResolutionGenerateId();
                newShipyard->cell = shipyard->cell;
                newShipyard->player = fleet->player;
                newShipyard->ships = fleet->ships - shipyard->ships;
                newShipyard->turnsControlled = 1;

                newShipyard->player->kore += fleet->kore;

                shipyardsById[newShipyard->id] = newShipyard.get();

                shipyardsById.erase(shipyardsById.find(shipyard->id));
                shipyard->remove();

                fleetsById.erase(fleetsById.find(fleet->id));
                fleet->remove();

                newShipyard->cell->shipyard = newShipyard.get();
                newShipyard->player->shipyards.push_back(std::move(newShipyard));
            }
        }
    }
}

void Board::turnResolutionFleetToFleetDamage() {
    std::unordered_map<Fleet *, std::vector<std::pair<Fleet *, int>>> incomingDamage;

    int directions[4][2] = {{1,  0},
                            {-1, 0},
                            {0,  1},
                            {0,  -1}};

    for (const auto &player : players) {
        for (const auto &fleet : player->fleets) {
            for (const auto &[dx, dy] : directions) {
                const auto &adjacentCell = cells.at(fleet->cell->x + dx, fleet->cell->y + dy);
                if (adjacentCell.fleets.empty()) {
                    continue;
                }

                const auto &attackingFleet = adjacentCell.fleets[0];
                if (attackingFleet->player == player.get()) {
                    continue;
                }

                incomingDamage[fleet.get()].push_back({attackingFleet, attackingFleet->ships});
            }
        }
    }

    if (incomingDamage.empty()) {
        return;
    }

    std::unordered_map<Cell *, std::vector<std::pair<Cell *, double>>> koreToDistribute;
    std::vector<Fleet *> deadFleets;

    for (const auto &[fleet, attackingFleets] : incomingDamage) {
        int totalDamage = 0;
        for (const auto &[attackingFleet, damage] : attackingFleets) {
            totalDamage += damage;
        }

        if (totalDamage >= fleet->ships) {
            fleet->cell->kore += fleet->kore / 2;

            double koreToSplit = fleet->kore / 2;
            for (const auto &[attackingFleet, damage] : attackingFleets) {
                double portion = koreToSplit * (((double) damage) / ((double) totalDamage));
                koreToDistribute[attackingFleet->cell].push_back({fleet->cell, portion});
            }

            deadFleets.push_back(fleet);
        } else {
            fleet->ships -= totalDamage;
        }
    }

    if (koreToDistribute.empty()) {
        return;
    }

    for (const auto &fleet : deadFleets) {
        fleetsById.erase(fleetsById.find(fleet->id));
        fleet->remove();
    }

    for (const auto &[cell, portions] : koreToDistribute) {
        if (cell->fleets.empty()) {
            for (const auto &[alternativeCell, kore] : portions) {
                alternativeCell->kore += kore;
            }
        } else {
            for (const auto &[alternativeCell, kore] : portions) {
                cell->fleets[0]->kore += kore;
            }
        }
    }
}

void Board::turnResolutionKoreMining() {
    for (const auto &player : players) {
        for (const auto &fleet : player->fleets) {
            if (fleet->cell->kore == 0.0) {
                continue;
            }

            double minedKore = fleet->cell->kore * fleet->getCollectionRate();

            fleet->kore += minedKore;
            fleet->cell->kore -= minedKore;
        }
    }
}

void Board::turnResolutionKoreRegeneration() {
    for (auto &cell : cells) {
        if (cell.shipyard != nullptr || !cell.fleets.empty()) {
            continue;
        }

        if (cell.kore < config.maxRegenCellKore) {
            cell.kore += cell.kore * config.regenRate;
        }
    }
}

void Board::turnResolutionEndTurn() {
    step++;
}

std::string Board::turnResolutionGenerateId() {
    return std::to_string(step + 1) + "-" + std::to_string(_idCounter++);
}
