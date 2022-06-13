#include <cmath>
#include <fstream>
#include <utility>

#include <strategy/FlightPlanDatabase.h>

FlightPlanDatabase::FlightPlanDatabase(const Configuration &config) : _boardSize(config.size) {
    loadPlans(config.agentDirectory / "data" / "target-plans.txt", _targetPlansByIndex, _targetPlansByIndexBySteps);
    loadPlans(config.agentDirectory / "data" / "convert-plans.txt", _convertPlansByIndex, _convertPlansByIndexBySteps);
}

std::vector<std::string> FlightPlanDatabase::getTargetPlans(const Cell &from, const Cell &to, int ships) const {
    return getPlans(from, to, ships, _targetPlansByIndex);
}

std::vector<std::string> FlightPlanDatabase::getTargetPlans(const Cell &from,
                                                            const Cell &to,
                                                            int ships,
                                                            int steps) const {
    return getPlans(from, to, ships, steps, _targetPlansByIndexBySteps);
}

std::vector<std::string> FlightPlanDatabase::getConvertPlans(const Cell &from, const Cell &to, int ships) const {
    return getPlans(from, to, ships, _convertPlansByIndex);
}

std::vector<std::string> FlightPlanDatabase::getConvertPlans(const Cell &from,
                                                             const Cell &to,
                                                             int ships,
                                                             int steps) const {
    return getPlans(from, to, ships, steps, _convertPlansByIndexBySteps);
}

void FlightPlanDatabase::loadPlans(const std::filesystem::path &file,
                                   std::vector<std::vector<std::string>> &plansByIndex,
                                   std::vector<std::unordered_map<int, std::vector<std::string>>> &plansByIndexBySteps) {
    plansByIndex.resize(_boardSize * _boardSize);
    plansByIndexBySteps.resize(_boardSize * _boardSize);

    std::ifstream stream(file);

    int chunkCount;
    stream >> chunkCount;

    for (int i = 0; i < chunkCount; i++) {
        int dx, dy, stepsOptions, uniquePlanCount;
        stream >> dx >> dy >> stepsOptions >> uniquePlanCount;

        int index = getIndex(dx, dy);
        auto &uniquePlans = plansByIndex[index];
        auto &plansByStepsMap = plansByIndexBySteps[index];

        uniquePlans.reserve(uniquePlanCount);

        for (int j = 0; j < stepsOptions; j++) {
            int steps, planCount;
            stream >> steps >> planCount;

            auto &plansByStepsVec = plansByStepsMap[steps];
            plansByStepsVec.reserve(planCount);

            for (int k = 0; k < planCount; k++) {
                bool isFirst;
                std::string plan;
                stream >> isFirst >> plan;

                if (isFirst) {
                    uniquePlans.push_back(plan);
                }

                plansByStepsVec.push_back(std::move(plan));
            }
        }
    }
}

std::vector<std::string> FlightPlanDatabase::getPlans(const Cell &from,
                                                      const Cell &to,
                                                      int ships,
                                                      const std::vector<std::vector<std::string>> &plansByIndex) const {
    return filterPlans(plansByIndex[getIndex(from, to)], ships);
}

std::vector<std::string> FlightPlanDatabase::getPlans(const Cell &from,
                                                      const Cell &to,
                                                      int ships,
                                                      int steps,
                                                      const std::vector<std::unordered_map<int, std::vector<std::string>>> &plansByIndexBySteps) const {
    if (from.distanceTo(to) > steps) {
        return {};
    }

    const auto &plansBySteps = plansByIndexBySteps[getIndex(from, to)];
    const auto &plans = plansBySteps.find(steps);

    if (plans == plansBySteps.end()) {
        return {};
    }

    return filterPlans(plans->second, ships);
}

std::vector<std::string> FlightPlanDatabase::filterPlans(const std::vector<std::string> &allPlans, int ships) const {
    int maxLength = std::floor(2.0 * std::log(ships)) + 1;

    std::vector<std::string> plans;
    plans.reserve(allPlans.size());

    for (const auto &plan : allPlans) {
        if (plan.size() <= maxLength) {
            plans.push_back(plan);
        }
    }

    return plans;
}

int FlightPlanDatabase::getIndex(int dx, int dy) const {
    if (dx < 0) {
        dx = _boardSize - ((-1 * dx) % _boardSize);
    }

    if (dx >= _boardSize) {
        dx %= _boardSize;
    }

    if (dy < 0) {
        dy = _boardSize - ((-1 * dy) % _boardSize);
    }

    if (dy >= _boardSize) {
        dy %= _boardSize;
    }

    return dy * _boardSize + dx;
}

int FlightPlanDatabase::getIndex(const Cell &from, const Cell &to) const {
    return getIndex(to.x - from.x, to.y - from.y);
}
