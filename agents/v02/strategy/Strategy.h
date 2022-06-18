#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <core/Board.h>
#include <strategy/StrategyComponent.h>

class Strategy {
    std::vector<std::unique_ptr<StrategyComponent>> _components;

public:
    Strategy();

    void run(Board &board);

    [[nodiscard]] std::unordered_map<std::string, double> getMetrics() const;

private:
    template<typename T>
    void registerComponent() {
        _components.push_back(std::make_unique<T>());
    }
};
