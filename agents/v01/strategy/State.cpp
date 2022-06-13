#include <strategy/State.h>

State::State(Board &board)
        : board(board),
          koreLeft(board.me().kore),
          availableShips(),
          savingForEnd(board.step >= board.config.episodeSteps - 50
                       && board.me().shipyards.size() >= board.opponent().shipyards.size()),
          timer() {
    for (const auto &shipyard : board.me().shipyards) {
        availableShips[shipyard->id] = shipyard->ships;
    }
}
