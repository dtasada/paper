#include <fcl/common/types.h>
#include <fcl/fcl.h>
#include <raylib.h>
#include <raymath.h>

enum class CellType { SOLID, FLUID, CUT_CELL, UNDEFINED };

#define N 32

#define IX(x, y, z)                                                     \
    std::clamp(int(x), 0, N - 1) + (std::clamp(int(y), 0, N - 1) * N) + \
        (std::clamp(int(z), 0, N - 1) * N * N)

static CellType state[N * N * N];

void voxelize(std::shared_ptr<fcl::BVHModel<fcl::OBBf>>& geom, Vector3 position) {
    geom->computeLocalAABB();
    fcl::CollisionObjectf obstacle_obj(
        geom, fcl::Transform3f(fcl::Translation3f(position.x, position.y, position.z)));

    for (int z = 0; z < N; z++) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                fcl::Vector3f cell_position(x, y, z);
                fcl::Vector3f cell_size(1.0f, 1.0f, 1.0f);

                auto cell_geometry = std::make_shared<fcl::Boxf>(cell_size);
                fcl::CollisionObjectf cell_obj(
                    cell_geometry, fcl::Transform3f(fcl::Translation3f(cell_position)));

                fcl::CollisionRequestf request;
                request.num_max_contacts = 128;
                fcl::CollisionResultf result;
                fcl::collide(&cell_obj, &obstacle_obj, request, result);

                if (result.isCollision()) {
                    state[IX(x, y, z)] = CellType::SOLID;

                    fcl::AABBf cell_aabb(cell_position, cell_position + cell_size);

                    bool fully_inside = true;
                    for (int i = 0; i < result.numContacts(); i++) {
                        const auto& contact = result.getContact(i);
                        std::cout << "Contact Point: " << contact.pos.transpose() << std::endl;
                        if (!cell_aabb.contain(contact.pos)) {
                            fully_inside = false;
                            break;
                        }
                    }

                    if (fully_inside) {
                        state[IX(x, y, z)] = CellType::SOLID;
                    } else {
                        state[IX(x, y, z)] = CellType::CUT_CELL;
                    }

                } else {
                    state[IX(x, y, z)] = CellType::FLUID;
                }
            }
        }
    }
}

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

int main() {
    InitWindow(1280, 720, "test");
    SetTargetFPS(60);
    HideCursor();
    DisableCursor();

    Camera3D camera = {
        .position = {20.0f, 20.0f, 20.0f},
        .target = {0.0f, 0.0f, 0.0f},
        .up = {0.0f, 1.0f, 0.0f},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    Model cube = LoadModelFromMesh(GenMeshCube(1, 1, 1));
    // Model plane = LoadModel("untitled_textures/untitled_texures.obj");
    Model plane = LoadModelFromMesh(GenMeshCube(8, 8, 8));

    Vector3 cell_position = {0.0f, 0.0f, 0.0f};
    float cell_size = 4.0f;

    Vector3 plane_pos = {16, 16, 16};
    std::shared_ptr<fcl::BVHModel<fcl::OBBf>> bvh = mesh_to_bvh(plane);

    voxelize(bvh, plane_pos);
    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginMode3D(camera);

        if (IsKeyDown(KEY_C)) UpdateCamera(&camera, CAMERA_THIRD_PERSON);

        ClearBackground(RAYWHITE);
        DrawGrid(100, 1.0f);
        DrawModel(cube, cell_position, cell_size, GREEN);
        DrawModel(plane, plane_pos, 1.0f, GREEN);

        for (float z = 0; z < N; z++) {
            for (float y = 0; y < N; y++) {
                for (float x = 0; x < N; x++) {
                    switch (state[IX(x, y, z)]) {
                        case CellType::SOLID:
                            DrawCube({x, y, z}, 1.0f, 1.0f, 1.0f, BLUE);
                            break;
                        case CellType::CUT_CELL:
                            DrawCube({x, y, z}, 1.0f, 1.0f, 1.0f, GREEN);
                            break;
                        case CellType::UNDEFINED:
                            DrawCube({x, y, z}, 1.0f, 1.0f, 1.0f, RED);
                            break;
                        case CellType::FLUID:
                            break;
                    }
                }
            }
        }

        EndMode3D();
        EndDrawing();
    }

    UnloadModel(cube);
    UnloadModel(plane);
    CloseWindow();
    return 0;
}
