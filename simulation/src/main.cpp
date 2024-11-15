#include <imgui.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "../include/engine/Engine.hpp"
#include "../include/engine/Fluid.hpp"
#include "../lib/rlImGui/rlImGui.h"

#define FPS 60.0f

int main(int argc, char* argv[]) {
    srand(time(nullptr));

    /* Raylib setup */
    InitWindow(1280, 720, "Fluid Grid");
    SetTargetFPS(FPS);
    SetExitKey(0);
    DisableCursor();
    // HideCursor();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    /* ImGui setup */
    rlImGuiSetup(true);

    /* Fluid sim setup */
    float diffusion = 0;         // Diffusion constant
    float dt = 1.0f;             // Timestep
    float viscosity = 0.000001;  // Viscosity constant

    Fluid fluid(24, 1.0f, diffusion, viscosity, dt);
    v3 containerSize(fluid.container_size * fluid.fluid_size);
    v3 containerCenter(containerSize * 0.5f);

    // fluid.add_cube(v3(4), 10);

    fluid.add_mesh(GenMeshCube(10, 10, 10), v3(4));

    bool cursor = false;
    Camera3D camera = {
        .position = containerSize,
        .target = containerCenter,
        .up = {0.0, 1.0, 0.0},
        .fovy = 45.0f,
        .projection = CAMERA_ORTHOGRAPHIC,
    };

    struct {
        bool show_cube = true;
        bool should_orthographic = true;
    } ui_settings;

    /* Main loop */
    while (!WindowShouldClose()) {
        /* Handle input */
        if (!cursor) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                DisableCursor();
                float x = 1, y = 1, z = 1;
                float amount = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 4 : 20;
                fluid.add_density({x, y, z}, 100);

                v3 vel(4, -9.81, 4);
                fluid.add_velocity({x, y, z}, v3(amount));
            }

            UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        }

        if (IsKeyPressed(KEY_F)) ToggleFullscreen();
        if (IsKeyPressed(KEY_R)) fluid.reset();
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (cursor) {
                DisableCursor();
                // HideCursor();
            } else {
                EnableCursor();
                // ShowCursor();
            }

            cursor = !cursor;
        }

        /* Update sim */
        fluid.step();

        // Calculate distance to camera for each cube. This is important for rendering purposes
        std::vector<Cell> cells;
        for (float z = 0; z < fluid.container_size; z++) {
            for (float y = 0; y < fluid.container_size; y++) {
                for (float x = 0; x < fluid.container_size; x++) {
                    v3 position(x, y, z);

                    float density = fluid.Density(position);
                    bool is_solid = fluid.Solid(position);
                    if (density > 0.01f || is_solid) {  // Skip cubes with very low density
                        v3 cube_position = position;
                        cube_position = cube_position * v3(fluid.fluid_size);
                        cube_position = cube_position + (fluid.fluid_size / 2);

                        // Calculate the squared distance to the camera
                        float dx = cube_position.x - camera.position.x;
                        float dy = cube_position.y - camera.position.y;
                        float dz = cube_position.z - camera.position.z;
                        float distanceSquared = dx * dx + dy * dy + dz * dz;

                        // Store cube data
                        cells.push_back({cube_position, density, is_solid, distanceSquared});
                    }

                    for (Mesh& mesh : fluid.meshes) {
                        if (check_collision_mesh(mesh, position)) {
                            fluid.set_solid(position, true);
                            break;
                        }
                    }
                }
            }
        }
        std::ranges::sort(cells, std::greater{}, &Cell::distanceSquared);

        /* Begin Drawing */
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);

        DrawCubeWiresV(containerCenter, containerSize, RED);

        // Render cubes
        BeginBlendMode(BLEND_ALPHA);
        for (const Cell& cell : cells) {
            if (!cell.is_solid) {
                // get color of cube
                float norm = std::min(cell.density / 100.0f, 1.0f);
                float hue = (1.0f - norm) * 0.66f * 360.0f;
                Color c = ColorFromHSV(hue, 1.0f, 1.0f);
                Color color = {c.r, c.g, c.b, (uint8_t)(norm * 255)};

                DrawCubeV(cell.position, v3(fluid.fluid_size), color);
            } else {
                if (ui_settings.show_cube) DrawCubeV(cell.position, v3(fluid.fluid_size), BROWN);
            }
        }
        for (size_t i = 0; i < fluid.meshes.size(); i++) {
            DrawModel(LoadModelFromMesh(fluid.meshes[i]), fluid.meshPositions[i], 1.0f, WHITE);
        }
        EndBlendMode();

        EndMode3D();

        DrawFPS(10, 10);
        DrawText(std::string("Position: ").append(v3(camera.position).to_string()).c_str(), 10, 30,
                 20, RAYWHITE);

        if (cursor) {
            /* Render UI */
            rlImGuiBegin();
            ImGui::Begin("Fluid Simulation");

            ImGui::Checkbox("Show cube", &ui_settings.show_cube);

            ImGui::Checkbox("Camera Orthograhic", &ui_settings.should_orthographic);
            camera.projection =
                ui_settings.should_orthographic ? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;

            ImGui::SliderFloat("Diffusion", &fluid.diffusion, 0.0f, 1.0f);

            ImGui::End();
            ImGui::Render();
            rlImGuiEnd();
        }

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
