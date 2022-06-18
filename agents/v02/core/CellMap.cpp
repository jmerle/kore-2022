#include <core/CellMap.h>

CellMap::CellMap(int size) : _cells(size * size), _size(size) {
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            auto &cell = _cells[cellToIndex(x, y)];
            cell.x = x;
            cell.y = y;
        }
    }
}

Cell &CellMap::at(int index) {
    return _cells[index];
}

const Cell &CellMap::at(int index) const {
    return _cells[index];
}

Cell &CellMap::at(int x, int y) {
    return _cells[cellToIndex(x, y)];
}

const Cell &CellMap::at(int x, int y) const {
    return _cells[cellToIndex(x, y)];
}

Cell &CellMap::at(const Cell &other) {
    return at(other.x, other.y);
}

const Cell &CellMap::at(const Cell &other) const {
    return at(other.x, other.y);
}

std::vector<Cell>::iterator CellMap::begin() {
    return _cells.begin();
}

std::vector<Cell>::const_iterator CellMap::begin() const {
    return _cells.begin();
}

std::vector<Cell>::iterator CellMap::end() {
    return _cells.end();
}

std::vector<Cell>::const_iterator CellMap::end() const {
    return _cells.end();
}

constexpr int CellMap::cellToIndex(int x, int y) const {
    if (x < 0) {
        x = _size - ((-1 * x) % _size);
    }

    if (x >= _size) {
        x %= _size;
    }

    if (y < 0) {
        y = _size - ((-1 * y) % _size);
    }

    if (y >= _size) {
        y %= _size;
    }

    return (_size - y - 1) * _size + x;
}
