#pragma once

#include <string>

#include <core/FlightPlan.h>

enum class ActionType {
    SPAWN,
    LAUNCH
};

struct Action {
    ActionType type;
    int ships;
    FlightPlan flightPlan;

    Action(ActionType type, int ships, FlightPlan flightPlan);

    [[nodiscard]] std::string toString() const;

    [[nodiscard]] static Action spawn(int ships);

    [[nodiscard]] static Action launch(int ships, const FlightPlan &flightPlan);
    [[nodiscard]] static Action launch(int ships, const std::string &flightPlan);

    [[nodiscard]] static Action parse(const std::string &action);
};
