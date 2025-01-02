#include "../../include/engine/engine.hpp"

#include <imgui.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <memory>
#include <vector>

Obstacle::Obstacle(v3 position, Model model, std::string identifier) {
    this->position = position;
    this->model = std::move(model);
    this->identifier = identifier;

    geom = mesh_to_bvh(model);
}

Obstacle::~Obstacle() { UnloadModel(model); }

std::shared_ptr<fcl::BVHModel<fcl::OBBf>> mesh_to_bvh(const Model& model) {
    std::shared_ptr<fcl::BVHModel<fcl::OBBf>> bvh = std::make_shared<fcl::BVHModel<fcl::OBBf>>();

    bvh->beginModel();
    for (int m = 0; m < model.meshCount; m++) {
        const Mesh& mesh = model.meshes[m];

        if (!mesh.vertices) throw std::runtime_error("Mesh vertices are missing!");
        if (mesh.triangleCount <= 0) throw std::runtime_error("Mesh has no triangles!");
        if (mesh.vertexCount <= 0) throw std::runtime_error("Mesh has no vertices!");

        std::vector<fcl::Vector3f> vertices;
        for (int i = 0; i < mesh.vertexCount; i++) {
            float x = mesh.vertices[i * 3];
            float y = mesh.vertices[i * 3 + 1];
            float z = mesh.vertices[i * 3 + 2];
            vertices.emplace_back(x, y, z);
        }

        std::vector<int> triangles;

        if (mesh.indices) {
            for (int i = 0; i < mesh.triangleCount * 3; i++) {
                triangles.push_back(mesh.indices[i]);
            }
        } else {
            for (int i = 0; i < mesh.vertexCount; i++) {
                triangles.push_back(i);
            }
        }

        for (size_t i = 0; i < triangles.size(); i += 3) {
            int idx1 = triangles[i];
            int idx2 = triangles[i + 1];
            int idx3 = triangles[i + 2];
            bvh->addTriangle(vertices[idx1], vertices[idx2], vertices[idx3]);
        }
    }

    bvh->endModel();
    return bvh;
}

bool drag_v3(const char* label, v3& v, float speed, float min, float max) {
    return ImGui::DragFloat3(label, &v.x, speed, min, max);
}
