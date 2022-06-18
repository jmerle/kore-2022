#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <pybind11/pybind11.h>

#include <core/Board.h>
#include <core/Cell.h>
#include <core/Configuration.h>
#include <core/Fleet.h>
#include <core/FlightPlan.h>
#include <core/Player.h>
#include <core/Shipyard.h>
#include <strategy/Strategy.h>

namespace py = pybind11;

std::optional<Strategy> strategy1;
std::optional<Strategy> strategy2;

Cell *indexToCell(Board &board, int index) {
    return &board.cells.at(index % board.config.size, board.config.size - index / board.config.size - 1);
}

void addPlayer(Board &board, const py::list &data, int id) {
    auto player = std::make_unique<Player>();
    player->id = id;
    player->kore = data[0].cast<double>();

    py::dict shipyards = data[1];
    for (const auto &[key, value] : shipyards) {
        auto info = value.cast<py::list>();

        auto shipyard = std::make_unique<Shipyard>();
        shipyard->id = key.cast<std::string>();
        shipyard->cell = indexToCell(board, info[0].cast<int>());
        shipyard->player = player.get();
        shipyard->ships = info[1].cast<int>();
        shipyard->turnsControlled = info[2].cast<int>();

        board.shipyardsById[shipyard->id] = shipyard.get();

        shipyard->cell->shipyard = shipyard.get();
        player->shipyards.push_back(std::move(shipyard));
    }

    py::dict fleets = data[2];
    for (const auto &[key, value] : fleets) {
        auto info = value.cast<py::list>();

        auto fleet = std::make_unique<Fleet>();
        fleet->id = key.cast<std::string>();
        fleet->cell = indexToCell(board, info[0].cast<int>());
        fleet->player = player.get();
        fleet->kore = info[1].cast<double>();
        fleet->ships = info[2].cast<int>();
        fleet->direction = static_cast<Direction>(info[3].cast<int>());
        fleet->flightPlan = FlightPlan::parse(info[4].cast<std::string>());

        board.fleetsById[fleet->id] = fleet.get();

        fleet->cell->fleets.push_back(fleet.get());
        player->fleets.push_back(std::move(fleet));
    }

    board.players.push_back(std::move(player));
}

py::dict agent(const py::dict &obs, const py::dict &config, const py::str &agentDirectory) {
    Configuration parsedConfig;
    parsedConfig.episodeSteps = config["episodeSteps"].cast<int>();
    parsedConfig.actTimeout = config["actTimeout"].cast<double>();
    parsedConfig.runTimeout = config["runTimeout"].cast<double>();
    parsedConfig.agentTimeout = config["agentTimeout"].cast<double>();
    parsedConfig.startingKore = config["startingKore"].cast<double>();
    parsedConfig.size = config["size"].cast<int>();
    parsedConfig.spawnCost = config["spawnCost"].cast<double>();
    parsedConfig.convertCost = config["convertCost"].cast<int>();
    parsedConfig.regenRate = config["regenRate"].cast<double>();
    parsedConfig.maxRegenCellKore = config["maxRegenCellKore"].cast<double>();
    parsedConfig.randomSeed = config["randomSeed"].cast<int>();
    parsedConfig.agentDirectory = std::filesystem::path(agentDirectory);

    Board board(parsedConfig);

    board.step = obs["step"].cast<int>();
    board.meIndex = obs["player"].cast<int>();
    board.remainingOverageTime = obs["remainingOverageTime"].cast<double>();

    py::list obsKore = obs["kore"];
    for (int i = 0, iMax = parsedConfig.size * parsedConfig.size; i < iMax; i++) {
        board.cells.at(i).kore = obsKore[i].cast<double>();
    }

    py::list obsPlayers = obs["players"];
    for (std::size_t i = 0; i < obsPlayers.size(); i++) {
        addPlayer(board, obsPlayers[i], i);
    }

    std::optional<Strategy> &strategy = board.meIndex == 0 ? strategy1 : strategy2;
    if (!strategy.has_value()) {
        strategy = Strategy(parsedConfig);
    }

    strategy->run(board);

    py::dict actions;
    for (const auto &shipyard : board.me().shipyards) {
        if (shipyard->action.has_value()) {
            actions[shipyard->id.c_str()] = shipyard->action->toString();
        }
    }

    for (const auto &[key, value] : strategy->getMetrics()) {
        actions[key.c_str()] = value;
    }

    return actions;
}

PYBIND11_MODULE(v01, m) {
    m.def("agent", &agent);
}
