#include <gtest/gtest.h>

#include <core/FlightPlan.h>

TEST(FlightPlanTest, Parse) {
    FlightPlan flightPlan = FlightPlan::parse("0N1E2S3W4N5E6S7W8C9C10");

    ASSERT_EQ(21, flightPlan.size());

    for (int i = 1; i < 16; i += 2) {
        EXPECT_EQ(FlightPlanPartType::TURN, flightPlan[i].type) << "i=" + std::to_string(i);
        EXPECT_EQ(static_cast<Direction>(((i - 1) / 2) % 4), flightPlan[i].direction) << "i=" + std::to_string(i);
    }

    for (int i = 0; i < 19; i += 2) {
        EXPECT_EQ(FlightPlanPartType::MOVE, flightPlan[i].type) << "i=" + std::to_string(i);
        EXPECT_EQ(i / 2, flightPlan[i].steps) << "i=" + std::to_string(i);
    }

    EXPECT_EQ(FlightPlanPartType::CONVERT, flightPlan[17].type);
    EXPECT_EQ(FlightPlanPartType::CONVERT, flightPlan[19].type);

    EXPECT_EQ(FlightPlanPartType::MOVE, flightPlan[20].type);
    EXPECT_EQ(10, flightPlan[20].steps);
}

TEST(FlightPlanTest, ToString) {
    EXPECT_EQ("0N1E2S3W4N5E6S7W8C9C10", FlightPlan::parse("0N1E2S3W4N5E6S7W8C9C10").toString());
}
