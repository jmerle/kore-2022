#pragma once

#include <string>
#include <deque>

#include <core/Direction.h>

enum class FlightPlanPartType {
    TURN,
    MOVE,
    CONVERT
};

struct FlightPlanPart {
    FlightPlanPartType type;
    Direction direction;
    int steps;

    static FlightPlanPart turn(Direction direction);
    static FlightPlanPart move(int steps);
    static FlightPlanPart convert();
};

struct FlightPlan : public std::deque<FlightPlanPart> {
    [[nodiscard]] std::string toString() const;

    [[nodiscard]] static FlightPlan parse(const std::string &flightPlan);
};
