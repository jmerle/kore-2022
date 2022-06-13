#pragma once

#include <string>
#include <unordered_map>

#include <core/Board.h>
#include <strategy/Timer.h>

struct State {
    Board &board;

    double koreLeft;
    std::unordered_map<std::string, int> availableShips;
    bool savingForEnd;

    Timer timer;

    explicit State(Board &board);
};
