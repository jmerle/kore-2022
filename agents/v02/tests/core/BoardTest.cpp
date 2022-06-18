#include <cstddef>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <core/Board.h>
#include <core/Player.h>
#include <tests/utilities.h>

namespace {
struct BoardTest : public testing::Test {
    void testNext(const nlohmann::json &data, std::size_t step) {
        Board actual = createBoard(data, step, true);
        Board expected = createBoard(data, step + 1, true);

        actual.next();

        assertBoardEquals(expected, actual);

        for (const auto &player : actual.players) {
            for (const auto &shipyard : player->shipyards) {
                EXPECT_FALSE(shipyard->action.has_value());
            }
        }
    }

    void assertBoardEquals(const Board &expected, const Board &actual) {
        EXPECT_EQ(expected.config.episodeSteps, actual.config.episodeSteps);
        EXPECT_EQ(expected.config.actTimeout, actual.config.actTimeout);
        EXPECT_EQ(expected.config.runTimeout, actual.config.runTimeout);
        EXPECT_EQ(expected.config.agentTimeout, actual.config.agentTimeout);
        EXPECT_EQ(expected.config.startingKore, actual.config.startingKore);
        ASSERT_EQ(expected.config.size, actual.config.size);
        EXPECT_EQ(expected.config.spawnCost, actual.config.spawnCost);
        EXPECT_EQ(expected.config.convertCost, actual.config.convertCost);
        EXPECT_EQ(expected.config.regenRate, actual.config.regenRate);
        EXPECT_EQ(expected.config.maxRegenCellKore, actual.config.maxRegenCellKore);
        EXPECT_EQ(expected.config.randomSeed, actual.config.randomSeed);

        EXPECT_EQ(expected.step, actual.step);
        EXPECT_EQ(expected.meIndex, actual.meIndex);

        for (int y = 0; y < expected.config.size; y++) {
            for (int x = 0; x < expected.config.size; x++) {
                auto params = "x=" + std::to_string(x) + ", y=" + std::to_string(y);
                EXPECT_NEAR(expected.cells.at(x, y).kore, actual.cells.at(x, y).kore, 0.001) << params;
            }
        }

        ASSERT_EQ(expected.players.size(), actual.players.size());
        for (std::size_t i = 0; i < expected.players.size(); i++) {
            assertPlayerEquals(*expected.players[i], *actual.players[i], i);
        }
    }

    void assertPlayerEquals(const Player &expected, const Player &actual, std::size_t who) {
        EXPECT_NEAR(expected.kore, actual.kore, 0.001) << "player=" + std::to_string(who);

        EXPECT_EQ(expected.shipyards.size(), actual.shipyards.size());
        for (const auto &expectedShipyard : expected.shipyards) {
            auto params = "player=" + std::to_string(who) + ", shipyard.id=" + expectedShipyard->id;
            bool actualFound = false;

            for (const auto &actualShipyard : actual.shipyards) {
                if (expectedShipyard->cell->x != actualShipyard->cell->x
                    || expectedShipyard->cell->y != actualShipyard->cell->y) {
                    continue;
                }

                actualFound = true;

                EXPECT_EQ(expectedShipyard->ships, actualShipyard->ships) << params;
                EXPECT_EQ(expectedShipyard->turnsControlled, actualShipyard->turnsControlled) << params;
            }

            EXPECT_TRUE(actualFound) << params;
        }

        EXPECT_EQ(expected.fleets.size(), actual.fleets.size());
        for (const auto &expectedFleet : expected.fleets) {
            auto params = "player=" + std::to_string(who) + ", fleet.id=" + expectedFleet->id;
            bool actualFound = false;

            for (const auto &actualFleet : actual.fleets) {
                if (expectedFleet->cell->x != actualFleet->cell->x || expectedFleet->cell->y != actualFleet->cell->y) {
                    continue;
                }

                actualFound = true;

                EXPECT_NEAR(expectedFleet->kore, actualFleet->kore, 0.001) << params;
                EXPECT_EQ(expectedFleet->ships, actualFleet->ships) << params;
                EXPECT_EQ(expectedFleet->direction, actualFleet->direction) << params;
                EXPECT_EQ(expectedFleet->flightPlan.toString(), actualFleet->flightPlan.toString()) << params;
            }

            EXPECT_TRUE(actualFound) << params;
        }
    }
};

#define CREATE_BOARD_TEST(id)                                                          \
struct BoardTest##id : BoardTest, public testing::WithParamInterface<int> {            \
};                                                                                     \
                                                                                       \
auto episodeData##id = parseDataFile(#id ".json");                                     \
                                                                                       \
TEST_P(BoardTest##id, Next) {                                                          \
    testNext(episodeData##id, GetParam());                                             \
}                                                                                      \
                                                                                       \
INSTANTIATE_TEST_SUITE_P(BoardTest,                                                    \
                         BoardTest##id,                                                \
                         testing::Range(0, (int) episodeData##id["steps"].size() - 1), \
                         [](const testing::TestParamInfo<int> &info) {                 \
                             auto from = std::to_string(info.param + 1);               \
                             auto to = std::to_string(info.param + 2);                 \
                             return "Step_" + from + "_to_" + to;                      \
                         });

CREATE_BOARD_TEST(36310051)
CREATE_BOARD_TEST(36854179)
CREATE_BOARD_TEST(36857057)
CREATE_BOARD_TEST(36857242)
CREATE_BOARD_TEST(36857473)
CREATE_BOARD_TEST(36857552)
CREATE_BOARD_TEST(36857623)
CREATE_BOARD_TEST(36857773)
CREATE_BOARD_TEST(36857827)
CREATE_BOARD_TEST(36858040)

#undef CREATE_BOARD_TEST
}
