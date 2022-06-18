#include <cstddef>
#include <stdexcept>

#include <core/FlightPlan.h>

FlightPlanPart FlightPlanPart::turn(Direction direction) {
    return {FlightPlanPartType::TURN, direction, 0};
}

FlightPlanPart FlightPlanPart::move(int steps) {
    return {FlightPlanPartType::MOVE, Direction::NORTH, steps};
}

FlightPlanPart FlightPlanPart::convert() {
    return {FlightPlanPartType::CONVERT, Direction::NORTH, 0};
}

std::string FlightPlan::toString() const {
    std::string str;

    for (const auto &part : *this) {
        switch (part.type) {
            case FlightPlanPartType::TURN:
                switch (part.direction) {
                    case Direction::NORTH:
                        str += 'N';
                        break;
                    case Direction::EAST:
                        str += 'E';
                        break;
                    case Direction::SOUTH:
                        str += 'S';
                        break;
                    case Direction::WEST:
                        str += 'W';
                        break;
                }
                break;
            case FlightPlanPartType::MOVE:
                str += std::to_string(part.steps);
                break;
            case FlightPlanPartType::CONVERT:
                str += 'C';
                break;
        }
    }

    return str;
}

FlightPlan FlightPlan::parse(const std::string &flightPlan) {
    FlightPlan parsedPlan;

    for (std::size_t i = 0; i < flightPlan.size(); i++) {
        char ch = flightPlan[i];

        switch (ch) {
            case 'N':
                parsedPlan.push_back(FlightPlanPart::turn(Direction::NORTH));
                break;
            case 'E':
                parsedPlan.push_back(FlightPlanPart::turn(Direction::EAST));
                break;
            case 'S':
                parsedPlan.push_back(FlightPlanPart::turn(Direction::SOUTH));
                break;
            case 'W':
                parsedPlan.push_back(FlightPlanPart::turn(Direction::WEST));
                break;
            case 'C':
                parsedPlan.push_back(FlightPlanPart::convert());
                break;
            default:
                if (ch >= '0' && ch <= '9') {
                    std::string steps;

                    for (std::size_t j = i; j < flightPlan.size(); i++, j++) {
                        char stepsCh = flightPlan[j];
                        if (stepsCh >= '0' && stepsCh <= '9') {
                            steps += stepsCh;
                        } else {
                            i--;
                            break;
                        }
                    }

                    parsedPlan.push_back(FlightPlanPart::move(std::atoi(steps.c_str())));
                } else {
                    throw std::invalid_argument("Invalid flight plan character: " + std::to_string(ch));
                }
        }
    }

    return parsedPlan;
}
