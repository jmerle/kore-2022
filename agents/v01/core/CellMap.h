#pragma once

#include <vector>

#include <core/Cell.h>

class CellMap {
    std::vector<Cell> _cells;
    int _size;

public:
    explicit CellMap(int size);

    CellMap(const CellMap &other) = default;

    [[nodiscard]] Cell &at(int index);
    [[nodiscard]] const Cell &at(int index) const;

    [[nodiscard]] Cell &at(int x, int y);
    [[nodiscard]] const Cell &at(int x, int y) const;

    [[nodiscard]] Cell &at(const Cell &other);
    [[nodiscard]] const Cell &at(const Cell &other) const;

    [[nodiscard]] std::vector<Cell>::iterator begin();
    [[nodiscard]] std::vector<Cell>::const_iterator begin() const;

    [[nodiscard]] std::vector<Cell>::iterator end();
    [[nodiscard]] std::vector<Cell>::const_iterator end() const;

private:
    [[nodiscard]] constexpr int cellToIndex(int x, int y) const;
};
