#include <cstddef>
#include <cstdlib>
#include <utility>

#include <core/Action.h>

Action::Action(ActionType type, int ships, FlightPlan flightPlan)
        : type(type), ships(ships), flightPlan(std::move(flightPlan)) {}

std::string Action::toString() const {
    if (type == ActionType::SPAWN) {
        return "SPAWN_" + std::to_string(ships);
    } else {
        return "LAUNCH_" + std::to_string(ships) + "_" + flightPlan.toString();
    }
}

Action Action::spawn(int ships) {
    return {ActionType::SPAWN, ships, {}};
}

Action Action::launch(int ships, const FlightPlan &flightPlan) {
    return {ActionType::LAUNCH, ships, flightPlan};
}

Action Action::launch(int ships, const std::string &flightPlan) {
    return launch(ships, FlightPlan::parse(flightPlan));
}

Action Action::parse(const std::string &action) {
    if (action[0] == 'S') {
        return spawn(std::atoi(action.substr(6).c_str()));
    } else {
        std::size_t separator1 = action.find('_');
        std::size_t separator2 = action.find('_', separator1 + 1);

        return launch(std::atoi(action.substr(separator1 + 1, separator2 - separator1 - 1).c_str()),
                      action.substr(separator2 + 1));
    }
}
