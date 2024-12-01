#include <imgui.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <sys/cdefs.h>

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
    HideCursor();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    /* ImGui setup */
    rlImGuiSetup(true);

    /* Fluid sim setup */
    float diffusion = 0;         // Diffusion constant
    float dt = 1.0f;             // Timestep
    float viscosity = 0.000001;  // Viscosity constant

    Fluid fluid(SIM_RES, 1.0f, diffusion, viscosity, dt);
    v3 containerSize(fluid.container_size * fluid.fluid_size);
    v3 containerCenter(containerSize * 0.5f);

    fluid.add_obstacle(v3(10.0f), v3(5.0f), LoadModel("simulation/resources/models/cube.obj"));

    bool cursor = false;
    Camera3D camera = {
        .position = containerSize,
        .target = containerCenter,
        .up = {0.0, 1.0, 0.0},
        .fovy = 45.0f,
        .projection = CAMERA_ORTHOGRAPHIC,
    };

    struct {
        bool show_models = true;
        bool should_orthographic = true;
        bool show_density_float = false;
        bool show_vel_arrows = false;
        bool gravity = false;
    } settings;

    /* Main loop */
    while (!WindowShouldClose()) {
        /* Handle input */
        if (!cursor) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                v3 position(1, 1, 1);
                float density_amount = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 100 : 200;
                fluid.add_density(position, density_amount);

                v3 velocity_amount(4, 4, 4);
                fluid.add_velocity(position, velocity_amount);
            }

            UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        }

        if (IsKeyPressed(KEY_F)) ToggleFullscreen();
        if (IsKeyPressed(KEY_R)) fluid.reset();
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (cursor) {
                DisableCursor();
                HideCursor();
            } else {
                EnableCursor();
                ShowCursor();
            }

            cursor = !cursor;
        }

        if (IsKeyDown(KEY_G)) {
            fluid.add_velocity(v3(12.0f, 12.0f, 12.0f), v3(0.0f, -9.81f, 0.0f));
        }

        /* Update sim */
        fluid.step();

        // Sort cell by distance to camera. This is important for backface rendering
        std::vector<Cell> cells;
        for (float z = 0.0f; z < fluid.container_size; z++) {
            for (float y = 0.0f; y < fluid.container_size; y++) {
                for (float x = 0.0f; x < fluid.container_size; x++) {
                    v3 position(x, y, z);

                    if (settings.gravity) {
                        fluid.add_velocity(position, v3(0.0f, -9.81f, 0.0f));
                    }

                    float density = fluid.get_density(position);

                    if (density > 0.01f ||
                        fluid.get_state(position) !=
                            CellType::FLUID) {  // Skip cubes with very low density
                        v3 cube_position = position;
                        cube_position = cube_position * v3(fluid.fluid_size);
                        cube_position = cube_position + (fluid.fluid_size / 2);

                        // Calculate the squared distance to the camera
                        float dx = cube_position.x - camera.position.x;
                        float dy = cube_position.y - camera.position.y;
                        float dz = cube_position.z - camera.position.z;
                        float distanceSquared = dx * dx + dy * dy + dz * dz;

                        // Store cube data
                        cells.push_back({cube_position, distanceSquared});
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
        if (settings.show_models) {
            for (Obstacle& obstacle : fluid.obstacles) {
                DrawModel(obstacle.model, obstacle.position, obstacle.size.x, RED);
            }
        }

        for (const Cell& cell : cells) {
            v3 position = cell.position;

            float density = fluid.get_density(cell.position);
            CellType state = fluid.get_state(cell.position);
            if (density > 0.01f) {
                if (settings.show_vel_arrows) {
                    BeginBlendMode(BLEND_SUBTRACT_COLORS);
                    DrawCylinderEx(position, position + (fluid.get_velocity(position) * 100),
                                   density / 100.f, density / 100.f, 10, RED);
                    BeginBlendMode(BLEND_ALPHA);
                } else {
                    // get color of cube
                    float norm = std::min(density / 100.0f, 1.0f);
                    float hue = (1.0f - norm) * 0.66f * 360.0f;
                    Color c = ColorFromHSV(hue, 1.0f, 1.0f);
                    Color color = {c.r, c.g, c.b, static_cast<uint8_t>(norm * 255)};

                    if (!settings.show_density_float) {
                        DrawCubeV(position, v3(fluid.fluid_size), color);
                    } else {
                        draw_text_3d(TextFormat("%.1f", density), position, fluid.fluid_size * 10,
                                     WHITE);
                    }
                }
            }

            if (state == CellType::SOLID) {
                DrawCubeV(position, v3(fluid.fluid_size), WHITE);
            }
            if (state == CellType::CUT_CELL) {
                DrawCubeV(position, v3(fluid.fluid_size), GREEN);
            }
        }
        EndBlendMode();

        EndMode3D();

        DrawFPS(10, 10);

        if (cursor) {
            /* Render UI */
            rlImGuiBegin();
            ImGui::Begin("Fluid Simulation");

            ImGui::Checkbox("Gravity", &settings.gravity);
            ImGui::Checkbox("Show models", &settings.show_models);
            ImGui::Checkbox("Show density with numbers", &settings.show_density_float);
            ImGui::Checkbox("Show velocity with arrows", &settings.show_vel_arrows);

            ImGui::Checkbox("Camera Orthograhic", &settings.should_orthographic);
            camera.projection =
                settings.should_orthographic ? CAMERA_ORTHOGRAPHIC : CAMERA_PERSPECTIVE;

            ImGui::SliderFloat("Diffusion", &fluid.diffusion, 0.0f, 0.0001f);

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
