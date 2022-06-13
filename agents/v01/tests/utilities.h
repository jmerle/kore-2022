#pragma once

#include <cstddef>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include <core/Action.h>
#include <core/Board.h>
#include <core/Cell.h>
#include <core/Configuration.h>
#include <core/Fleet.h>
#include <core/FlightPlan.h>
#include <core/Player.h>
#include <core/Shipyard.h>

nlohmann::json parseDataFile(const std::string &dataFile) {
    std::ifstream stream("test-data/" + dataFile, std::ios::in);
    return nlohmann::json::parse(stream);
}

Cell *indexToCell(Board &board, int index) {
    return &board.cells.at(index % board.config.size, board.config.size - index / board.config.size - 1);
}

void addPlayer(Board &board, const nlohmann::json &playerData, const nlohmann::json &actionData, int id) {
    auto player = std::make_unique<Player>();
    player->id = id;
    player->kore = playerData[0];

    for (const auto &item : playerData[1].items()) {
        auto shipyard = std::make_unique<Shipyard>();
        shipyard->id = item.key();
        shipyard->cell = indexToCell(board, item.value()[0]);
        shipyard->player = player.get();
        shipyard->ships = item.value()[1];
        shipyard->turnsControlled = item.value()[2];

        if (actionData.contains(shipyard->id)) {
            shipyard->action = Action::parse(actionData[shipyard->id]);
        }

        board.shipyardsById[shipyard->id] = shipyard.get();

        shipyard->cell->shipyard = shipyard.get();
        player->shipyards.push_back(std::move(shipyard));
    }

    for (const auto &item : playerData[2].items()) {
        auto fleet = std::make_unique<Fleet>();
        fleet->id = item.key();
        fleet->cell = indexToCell(board, item.value()[0]);
        fleet->player = player.get();
        fleet->kore = item.value()[1];
        fleet->ships = item.value()[2];
        fleet->direction = item.value()[3];
        fleet->flightPlan = FlightPlan::parse(item.value()[4]);

        board.fleetsById[fleet->id] = fleet.get();

        fleet->cell->fleets.push_back(fleet.get());
        player->fleets.push_back(std::move(fleet));
    }

    board.players.push_back(std::move(player));
}

Board createBoard(const nlohmann::json &data, std::size_t step) {
    const auto &configObj = data["configuration"];

    Configuration config;
    config.episodeSteps = configObj["episodeSteps"];
    config.actTimeout = configObj["actTimeout"];
    config.runTimeout = configObj["runTimeout"];
    config.agentTimeout = configObj["agentTimeout"];
    config.startingKore = configObj["startingKore"];
    config.size = configObj["size"];
    config.spawnCost = configObj["spawnCost"];
    config.convertCost = configObj["convertCost"];
    config.regenRate = configObj["regenRate"];
    config.maxRegenCellKore = configObj["maxRegenCellKore"];
    config.randomSeed = configObj["randomSeed"];

    Board board(config);

    const auto &observation = data["steps"][step][0]["observation"];

    board.step = observation["step"];
    board.meIndex = 0;
    board.remainingOverageTime = observation["remainingOverageTime"];

    for (int i = 0, iMax = config.size * config.size; i < iMax; i++) {
        board.cells.at(i).kore = observation["kore"][i];
    }

    for (std::size_t i = 0; i < observation["players"].size(); i++) {
        nlohmann::json actionData;
        if (step < data["steps"].size() - 1) {
            actionData = data["steps"][step + 1][i]["action"];
        }

        addPlayer(board, observation["players"][i], actionData, i);
    }

    return board;
}
