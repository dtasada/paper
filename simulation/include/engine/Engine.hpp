#pragma once
#include <raylib.h>
#include <raymath.h>

#undef PI

#include <fcl/common/types.h>
#include <fcl/fcl.h>
#include <fcl/geometry/bvh/BVH_model.h>
#include <fcl/math/bv/OBB.h>
#include <fcl/math/bv/OBBRSS.h>
#include <fcl/math/triangle.h>

#include "v3.hpp"

struct Cell {
    v3 position;
    float distanceSquared;  // Distance from the camera (squared)
};

struct Obstacle {
    v3 position;
    Model model;

    std::shared_ptr<fcl::BVHModel<fcl::OBBf>> geom;

    Obstacle(v3 position, Model model);
    ~Obstacle();
};

int constrain(int val, int low, int high);
std::shared_ptr<fcl::BVHModel<fcl::OBBf>> mesh_to_bvh(const Mesh& mesh);
