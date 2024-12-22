#include <imgui.h>
#include <raylib.h>
#include <raymath.h>
#include <rlImGui.h>
#include <rlgl.h>
#include <sys/cdefs.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <toml++/toml.hpp>
#include <vector>

#include "../include/engine/Engine.hpp"
#include "../include/engine/Fluid.hpp"

#define FPS 60.0f

int main(int argc, char* argv[]) {
    srand(time(nullptr));

    /* Raylib setup */
    InitWindow(1280, 720, "Fluid Grid");
    SetTargetFPS(FPS);
    SetExitKey(KEY_Q);
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

    /* Parse config file */
    try {
        auto config = toml::parse_file("config.toml");
        if (!config.empty())
            for (const auto& node : *config["obstacle"].as_array()) {
                const auto& obstacle = *node.as_table();
                const auto& size = obstacle["size"].value_or(1.0f);

                const auto& position_array = *obstacle["position"].as_array();
                v3 position(position_array[0].value_or(0.0), position_array[1].value_or(0.0),
                            position_array[2].value_or(0.0));

                // Parse model
                Model model = LoadModel(obstacle["model"].value_or(""));

                fluid.add_obstacle(position, size, model);
            }
    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse config file: " << err.what() << std::endl;
    }

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
        bool show_vel_arrows = false;
        bool show_bounds = false;
        bool show_cell_borders = false;
        bool show_bounds_solid = false;
        bool show_bounds_cut_cell = false;
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

        /* Update sim */
        fluid.step();

        /* Begin Drawing */
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);

        /* Sort cell by distance to camera. This is important for backface rendering */
        std::vector<Cell> cells;
        for (float z = 0.0f; z < fluid.container_size; z++) {
            for (float y = 0.0f; y < fluid.container_size; y++) {
                for (float x = 0.0f; x < fluid.container_size; x++) {
                    v3 position(x, y, z);

                    if (settings.show_cell_borders) {
                        DrawCubeWiresV(position + fluid.fluid_size / 2, v3(fluid.fluid_size), RED);
                    }

                    if (fluid.get_density(position) > 0.01f ||
                        fluid.get_state(position) != CellType::FLUID) {
                        // Skip cubes with very low density
                        v3 cube_position = position;
                        cube_position = cube_position * v3(fluid.fluid_size);
                        cube_position = cube_position + (fluid.fluid_size / 2);

                        float dx = cube_position.x - camera.position.x;
                        float dy = cube_position.y - camera.position.y;
                        float dz = cube_position.z - camera.position.z;
                        float distanceSquared = dx * dx + dy * dy + dz * dz;

                        cells.push_back({cube_position, distanceSquared});
                    }
                }
            }
        }

        std::ranges::sort(cells, std::greater{}, &Cell::distanceSquared);

        DrawCubeWiresV(containerCenter, containerSize, RED);

        // Render cubes
        BeginBlendMode(BLEND_ALPHA);
        if (settings.show_models) {
            for (Obstacle& obstacle : fluid.obstacles) {
                DrawModel(obstacle.model, obstacle.position, obstacle.size, RED);
            }
        }

        for (const Cell& cell : cells) {
            v3 position = cell.position;

            float density = fluid.get_density(position);
            CellType state = fluid.get_state(position);

            switch (state) {
                case CellType::SOLID:
                    if (settings.show_bounds_solid)
                        DrawCubeV(position, v3(fluid.fluid_size), BLUE);
                    break;
                case CellType::CUT_CELL:
                    if (settings.show_bounds_cut_cell)
                        DrawCubeV(position, v3(fluid.fluid_size), GREEN);
                    break;
                case CellType::FLUID: {
                    if (density > 0.01f) {
                        if (settings.show_vel_arrows) {
                            BeginBlendMode(BLEND_SUBTRACT_COLORS);
                            DrawCylinderEx(position,
                                           position + (fluid.get_velocity(position) * 100),
                                           density / 100.f, density / 100.f, 10, RED);
                            BeginBlendMode(BLEND_ALPHA);
                        } else {
                            // get color of cube
                            float norm = std::min(density / 100.0f, 1.0f);
                            float hue = (1.0f - norm) * 0.66f * 360.0f;
                            Color c = ColorFromHSV(hue, 1.0f, 1.0f);
                            Color color = {c.r, c.g, c.b, static_cast<uint8_t>(norm * 255)};

                            DrawCubeV(position, v3(fluid.fluid_size), color);
                        }
                    }
                } break;
            }
        }

        EndBlendMode();

        EndMode3D();

        DrawFPS(10, 10);

        if (cursor) {
            /* Render UI */
            rlImGuiBegin();
            ImGui::Begin("Fluid Simulation");

            ImGui::Checkbox("Show models", &settings.show_models);
            ImGui::Checkbox("Show velocity with arrows", &settings.show_vel_arrows);
            // ImGui::Checkbox("Show bounds", &settings.show_bounds);
            ImGui::Checkbox("Show bounds SOLID", &settings.show_bounds_solid);
            ImGui::Checkbox("Show bounds CUT_CELL", &settings.show_bounds_cut_cell);
            ImGui::Checkbox("Show cell borders", &settings.show_cell_borders);

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
