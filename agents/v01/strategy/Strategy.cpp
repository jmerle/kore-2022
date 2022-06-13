#include <algorithm>

#include <core/Shipyard.h>
#include <strategy/State.h>
#include <strategy/Strategy.h>
#include <strategy/components/AttackFleetComponent.h>
#include <strategy/components/AttackShipyardComponent.h>
#include <strategy/components/DefendComponent.h>
#include <strategy/components/ExpandComponent.h>
#include <strategy/components/MineComponent.h>
#include <strategy/components/SpawnGreedyComponent.h>
#include <strategy/components/SpawnNormalComponent.h>

Strategy::Strategy(const Configuration &config) : _flightPlanDatabase(std::make_unique<FlightPlanDatabase>(config)) {
    registerComponent<DefendComponent>();
    registerComponent<AttackShipyardComponent>();
    registerComponent<AttackFleetComponent>();
    registerComponent<ExpandComponent>();
    registerComponent<SpawnGreedyComponent>();
    registerComponent<MineComponent>();
    registerComponent<SpawnNormalComponent>();
}

void Strategy::run(Board &board) {
    Board::COPY_CALLS = 0;
    Board::NEXT_CALLS = 0;

    std::sort(board.me().shipyards.begin(),
              board.me().shipyards.end(),
              [](const std::unique_ptr<Shipyard> &a, const std::unique_ptr<Shipyard> &b) {
                  return a->turnsControlled < b->turnsControlled;
              });

    State state(board);

    for (const auto &component : _components) {
        component->run(state);
    }
}

std::unordered_map<std::string, double> Strategy::getMetrics() const {
    return {
            {"boardCopyCalls", Board::COPY_CALLS},
            {"boardNextCalls", Board::NEXT_CALLS}
    };
}
