#pragma once

#include <strategy/State.h>

class StrategyComponent {
public:
    virtual ~StrategyComponent() = default;

    virtual void run(State &state) = 0;
};
