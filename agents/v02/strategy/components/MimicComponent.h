#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <core/Action.h>
#include <core/Board.h>
#include <core/Cell.h>
#include <core/Direction.h>
#include <strategy/State.h>
#include <strategy/StrategyComponent.h>

class MimicComponent : public StrategyComponent {
    std::unique_ptr<Board> _previousBoard;

public:
    void run(State &state) override;

    [[nodiscard]] std::unordered_map<std::string, Action> getPreviousActions(const Board &previousBoard,
                                                                             const Board &currentBoard) const;

    [[nodiscard]] Direction directionBetween(const Cell &from, const Cell &to) const;
};
