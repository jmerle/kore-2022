#pragma once

#include <core/Board.h>
#include <strategy/Timer.h>

struct State {
    Board &board;
    Timer timer;

    explicit State(Board &board);
};
