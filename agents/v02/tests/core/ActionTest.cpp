#include <gtest/gtest.h>

#include <core/Action.h>
#include <core/FlightPlan.h>

TEST(ActionTest, ParseSpawn) {
    Action action = Action::parse("SPAWN_42");

    EXPECT_EQ(ActionType::SPAWN, action.type);
    EXPECT_EQ(42, action.ships);
}

TEST(ActionTest, ParseLaunch) {
    Action action = Action::parse("LAUNCH_42_N5CE5S5W");

    EXPECT_EQ(ActionType::LAUNCH, action.type);
    EXPECT_EQ(42, action.ships);

    ASSERT_EQ(8, action.flightPlan.size());

    EXPECT_EQ(FlightPlanPartType::TURN, action.flightPlan[0].type);
    EXPECT_EQ(Direction::NORTH, action.flightPlan[0].direction);

    EXPECT_EQ(FlightPlanPartType::MOVE, action.flightPlan[1].type);
    EXPECT_EQ(5, action.flightPlan[1].steps);

    EXPECT_EQ(FlightPlanPartType::CONVERT, action.flightPlan[2].type);

    EXPECT_EQ(FlightPlanPartType::TURN, action.flightPlan[3].type);
    EXPECT_EQ(Direction::EAST, action.flightPlan[3].direction);

    EXPECT_EQ(FlightPlanPartType::MOVE, action.flightPlan[4].type);
    EXPECT_EQ(5, action.flightPlan[4].steps);

    EXPECT_EQ(FlightPlanPartType::TURN, action.flightPlan[5].type);
    EXPECT_EQ(Direction::SOUTH, action.flightPlan[5].direction);

    EXPECT_EQ(FlightPlanPartType::MOVE, action.flightPlan[6].type);
    EXPECT_EQ(5, action.flightPlan[6].steps);

    EXPECT_EQ(FlightPlanPartType::TURN, action.flightPlan[7].type);
    EXPECT_EQ(Direction::WEST, action.flightPlan[7].direction);
}

TEST(ActionTest, ToStringSpawn) {
    Action action = Action::spawn(42);

    EXPECT_EQ("SPAWN_42", action.toString());
}

TEST(ActionTest, ToStringLaunch) {
    Action action = Action::launch(42, "N5CE5S5W");

    EXPECT_EQ("LAUNCH_42_N5CE5S5W", action.toString());
}
