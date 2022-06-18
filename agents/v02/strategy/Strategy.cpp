#include <strategy/State.h>
#include <strategy/Strategy.h>
#include <strategy/components/MimicComponent.h>

Strategy::Strategy() {
    registerComponent<MimicComponent>();
}

void Strategy::run(Board &board) {
    Board::COPY_CALLS = 0;
    Board::NEXT_CALLS = 0;

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
