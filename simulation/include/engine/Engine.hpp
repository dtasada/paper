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
    float distance_squared;  // Distance from the camera (squared)
};

struct Obstacle {
    v3 position;
    v3 scaling;
    Model model;
    bool enabled;
    std::string identifier;

    std::shared_ptr<fcl::BVHModel<fcl::OBBf>> geom;

    Obstacle(v3 position, v3 scaling, Model model, bool enabled, std::string identifier);
    ~Obstacle();
};

std::shared_ptr<fcl::BVHModel<fcl::OBBf>> mesh_to_bvh(const Model& mesh);

bool drag_v3(const char* label, v3& v, float speed, float min, float max);
