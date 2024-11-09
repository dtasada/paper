#include <cmath>
#include <cstddef>
#include <vector>

#include "Engine.hpp"

class Grid {
   private:
    size_t N;
    std::vector<float> data;

   public:
    Grid(size_t size);

    float& operator()(size_t x, size_t y, size_t z);
    const float& operator()(size_t x, size_t y, size_t z) const;

    float& operator()(const v3& vec);

    size_t size() const;
};
