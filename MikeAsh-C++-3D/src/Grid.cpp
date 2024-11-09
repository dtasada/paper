#include "../include/Grid.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

Grid::Grid(size_t size) : N(size), data(pow(size, 3), 0.0f) {}

float& Grid::operator()(size_t x, size_t y, size_t z) { return data[x + (y * N) + (z * N * N)]; }
const float& Grid::operator()(size_t x, size_t y, size_t z) const {
    // return data[x + (y * N) + (z * N * N)];
    return (*this)(x, y, z);
}

float& Grid::operator()(const v3& vec) {
    size_t x = std::clamp(static_cast<size_t>(vec.x), size_t(0), N - 1);
    size_t y = std::clamp(static_cast<size_t>(vec.y), size_t(0), N - 1);
    size_t z = std::clamp(static_cast<size_t>(vec.z), size_t(0), N - 1);
    return (*this)(x, y, z);
}

size_t Grid::size() const { return N; }
