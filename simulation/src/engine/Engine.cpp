#include "../../include/engine/Engine.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <algorithm>
#include <memory>
#include <vector>

Obstacle::Obstacle(v3 position, float size, Model model) {
    this->position = position;
    this->size = size;
    this->model = model;

    geom = mesh_to_bvh(model.meshes[0]);
}

Obstacle::~Obstacle() { UnloadModel(model); }

std::unique_ptr<fcl::BVHModel<fcl::OBBf>> mesh_to_bvh(const Mesh& mesh) {
    std::unique_ptr<fcl::BVHModel<fcl::OBBf>> model = std::make_unique<fcl::BVHModel<fcl::OBBf>>();

    if (!mesh.vertices) {
        throw std::runtime_error("Mesh vertices are missing!");
    }

    std::vector<fcl::Vector3f> vertices;
    for (int i = 0; i < mesh.vertexCount; i++) {
        float x = mesh.vertices[i * 3];
        float y = mesh.vertices[i * 3 + 1];
        float z = mesh.vertices[i * 3 + 2];
        vertices.emplace_back(x, y, z);
    }

    std::vector<int> triangles;
    for (int i = 0; i < mesh.triangleCount * 3; i++) {
        triangles.push_back(mesh.indices[i]);
    }

    model->beginModel();
    for (size_t i = 0; i < triangles.size(); i += 3) {
        int idx1 = triangles[i];
        int idx2 = triangles[i + 1];
        int idx3 = triangles[i + 2];
        model->addTriangle(vertices[idx1], vertices[idx2], vertices[idx3]);
    }
    model->endModel();

    return model;
}

int constrain(int val, int low, int high) { return std::min(std::max(val, low), high); }
