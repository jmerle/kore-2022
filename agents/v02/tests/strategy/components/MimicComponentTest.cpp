#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <core/Action.h>
#include <core/Board.h>
#include <strategy/components/MimicComponent.h>
#include <tests/utilities.h>

namespace {
struct MimicComponentTest : public testing::Test {
    void testGetPreviousActions(const nlohmann::json &data, std::size_t step) {
        Board previousBoard1 = createBoard(data, step, false);
        Board previousBoard2 = createBoard(data, step, true);
        Board currentBoard = createBoard(data, step + 1, false);

        if (hasInvalidActions(previousBoard2)) {
            GTEST_SKIP() << "Test data has invalid actions";
        }

        MimicComponent component;
        const auto &previousActions = component.getPreviousActions(previousBoard1, currentBoard);

        for (const auto &[shipyardId, shipyard] : previousBoard2.shipyardsById) {
            auto it = previousActions.find(shipyardId);
            if (shipyard->action.has_value()) {
                ASSERT_TRUE(it != previousActions.end());

                EXPECT_EQ(shipyard->action->type, it->second.type);
                EXPECT_EQ(shipyard->action->ships, it->second.ships);

                ASSERT_EQ(shipyard->action->flightPlan.size(), it->second.flightPlan.size());
                for (int i = 0; i < shipyard->action->flightPlan.size(); i++) {
                    EXPECT_EQ(shipyard->action->flightPlan[i].type, it->second.flightPlan[i].type);
                    EXPECT_EQ(shipyard->action->flightPlan[i].direction, it->second.flightPlan[i].direction);
                    EXPECT_EQ(shipyard->action->flightPlan[i].steps, it->second.flightPlan[i].steps);
                }
            } else {
                if (it != previousActions.end()) {
                    EXPECT_EQ(ActionType::SPAWN, it->second.type);
                    EXPECT_EQ(0, it->second.ships);
                }
            }
        }
    }

private:
    [[nodiscard]] bool hasInvalidActions(const Board &board) const {
        for (const auto &player : board.players) {
            double koreLeft = player->kore;
            for (const auto &shipyard : player->shipyards) {
                if (!shipyard->action.has_value()) {
                    continue;
                }

                if (shipyard->action->type == ActionType::SPAWN) {
                    koreLeft -= shipyard->ships * board.config.spawnCost;
                } else {
                    if (shipyard->ships < shipyard->action->ships) {
                        return true;
                    }
                }
            }

            if (koreLeft < 0) {
                return true;
            }
        }

        return false;
    }
};

#define CREATE_MIMIC_COMPONENT_TEST(id)                                                       \
struct MimicComponentTest##id : MimicComponentTest, public testing::WithParamInterface<int> { \
};                                                                                            \
                                                                                              \
auto episodeData##id = parseDataFile(#id ".json");                                            \
                                                                                              \
TEST_P(MimicComponentTest##id, GetPreviousActions) {                                          \
    testGetPreviousActions(episodeData##id, GetParam());                                      \
}                                                                                             \
                                                                                              \
INSTANTIATE_TEST_SUITE_P(MimicComponentTest,                                                  \
                         MimicComponentTest##id,                                              \
                         testing::Range(0, (int) episodeData##id["steps"].size() - 1),        \
                         [](const testing::TestParamInfo<int> &info) {                        \
                             auto from = std::to_string(info.param + 1);                      \
                             auto to = std::to_string(info.param + 2);                        \
                             return "Step_" + from + "_to_" + to;                             \
                         });

CREATE_MIMIC_COMPONENT_TEST(36310051)
CREATE_MIMIC_COMPONENT_TEST(36854179)
CREATE_MIMIC_COMPONENT_TEST(36857057)
CREATE_MIMIC_COMPONENT_TEST(36857242)
CREATE_MIMIC_COMPONENT_TEST(36857473)
CREATE_MIMIC_COMPONENT_TEST(36857552)
CREATE_MIMIC_COMPONENT_TEST(36857623)
CREATE_MIMIC_COMPONENT_TEST(36857773)
CREATE_MIMIC_COMPONENT_TEST(36857827)
CREATE_MIMIC_COMPONENT_TEST(36858040)

#undef CREATE_MIMIC_COMPONENT_TEST
}
