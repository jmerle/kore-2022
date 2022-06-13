#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <core/Cell.h>
#include <core/Configuration.h>

class FlightPlanDatabase {
    int _boardSize;

    std::vector<std::vector<std::string>> _targetPlansByIndex;
    std::vector<std::unordered_map<int, std::vector<std::string>>> _targetPlansByIndexBySteps;

    std::vector<std::vector<std::string>> _convertPlansByIndex;
    std::vector<std::unordered_map<int, std::vector<std::string>>> _convertPlansByIndexBySteps;

public:
    explicit FlightPlanDatabase(const Configuration &config);

    [[nodiscard]] std::vector<std::string> getTargetPlans(const Cell &from, const Cell &to, int ships) const;
    [[nodiscard]] std::vector<std::string> getTargetPlans(const Cell &from,
                                                          const Cell &to,
                                                          int ships,
                                                          int steps) const;

    [[nodiscard]] std::vector<std::string> getConvertPlans(const Cell &from, const Cell &to, int ships) const;
    [[nodiscard]] std::vector<std::string> getConvertPlans(const Cell &from,
                                                           const Cell &to,
                                                           int ships,
                                                           int steps) const;

private:
    void loadPlans(const std::filesystem::path &file,
                   std::vector<std::vector<std::string>> &plansByIndex,
                   std::vector<std::unordered_map<int, std::vector<std::string>>> &plansByIndexBySteps);

    [[nodiscard]] std::vector<std::string> getPlans(const Cell &from,
                                                    const Cell &to,
                                                    int ships,
                                                    const std::vector<std::vector<std::string>> &plansByIndex) const;

    [[nodiscard]] std::vector<std::string> getPlans(const Cell &from,
                                                    const Cell &to,
                                                    int ships,
                                                    int steps,
                                                    const std::vector<std::unordered_map<int, std::vector<std::string>>> &plansByIndexBySteps) const;

    [[nodiscard]] std::vector<std::string> filterPlans(const std::vector<std::string> &allPlans, int ships) const;

    [[nodiscard]] int getIndex(int dx, int dy) const;
    [[nodiscard]] int getIndex(const Cell &from, const Cell &to) const;
};
