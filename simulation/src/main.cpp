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

#include "../include/engine/Fluid.hpp"
#include "../include/engine/engine.hpp"

int main(int argc, char* argv[]) {
    srand(time(nullptr));

    /* Raylib setup */
    InitWindow(1280, 720, "Fluid Grid");
    SetTargetFPS(60);
    SetExitKey(KEY_Q);
    DisableCursor();
    HideCursor();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);

    /* ImGui setup */
    rlImGuiSetup(true);

    // ui settings
    struct {
        bool show_models = true;
        bool camera_free = false;
        bool show_vel_arrows = false;
        bool show_bounds = false;
        bool show_cell_borders = false;
        bool show_bounds_solid = false;
        bool show_bounds_cut_cell = false;
        bool render_low_density = false;

        v3 insert_position;
        v3 insert_velocity;
    } settings;

    /* Parse config file */
    Fluid* fluid = nullptr;
    try {
        auto config = toml::parse_file("config.toml");
        if (!config.empty()) {
            fluid = new Fluid(config["settings"]["resolution"].value_or(24),
                              config["settings"]["scaling"].value_or(1.0f),
                              config["settings"]["diffusion"].value_or(0.0f),
                              config["settings"]["viscosity"].value_or(0.000001f),
                              config["settings"]["dt"].value_or(1.0f));

            const auto& insert_position = *config["settings"]["insert_position"].as_array();
            settings.insert_position =
                v3(insert_position[0].value_or(1.0f), insert_position[1].value_or(1.0f),
                   insert_position[2].value_or(1.0f));

            const auto& insert_velocity = *config["settings"]["insert_velocity"].as_array();
            settings.insert_velocity =
                v3(insert_velocity[0].value_or(4.0f), insert_velocity[1].value_or(4.0f),
                   insert_velocity[2].value_or(4.0f));

            for (const auto& node : *config["obstacle"].as_array()) {
                const auto& obstacle_table = *node.as_table();

                const auto& position_array = *obstacle_table["position"].as_array();
                v3 position(position_array[0].value_or(0.0), position_array[1].value_or(0.0),
                            position_array[2].value_or(0.0));

                std::unique_ptr<Obstacle> obstacle = std::make_unique<Obstacle>(
                    position,
                    LoadModel(
                        obstacle_table["model"].value_or("No .obj file given in config.toml")),
                    obstacle_table["identifier"].value_or(""));
                fluid->add_obstacle(std::move(obstacle));
            }
        }
    } catch (const toml::parse_error& err) {
        std::cerr << "Failed to parse config file: " << err.what() << std::endl;
    }

    v3 container_size(fluid->container_size * fluid->scaling);
    v3 container_center(container_size * 0.5f);

    bool cursor = false;
    Camera3D camera = {
        .position = container_size,
        .target = container_center,
        .up = {0.0, 1.0, 0.0},
        .fovy = 45.0f,
        .projection = CAMERA_ORTHOGRAPHIC,
    };

    /* Main loop */
    while (!WindowShouldClose()) {
        /* Handle input */
        if (!cursor) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                float density_amount = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 100 : 200;
                fluid->add_density(settings.insert_position, density_amount);

                fluid->add_velocity(settings.insert_position, settings.insert_velocity);
            }

            UpdateCamera(&camera, settings.camera_free ? CAMERA_FREE : CAMERA_THIRD_PERSON);
        }

        if (IsKeyPressed(KEY_F)) ToggleFullscreen();
        if (IsKeyPressed(KEY_R)) fluid->reset();
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
        fluid->step();

        /* Begin Drawing */
        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode3D(camera);

        // Sort cell by distance to camera. This is important for backface rendering
        std::vector<Cell> cells;
        for (float z = 0.0f; z < fluid->container_size; z++) {
            for (float y = 0.0f; y < fluid->container_size; y++) {
                for (float x = 0.0f; x < fluid->container_size; x++) {
                    v3 position(x, y, z);

                    if (settings.show_cell_borders) {
                        DrawCubeWiresV(position * fluid->scaling + fluid->scaling / 2,
                                       v3(fluid->scaling), RED);
                    }

                    // Skip cubes with very low density
                    if (fluid->get_density(position) > 0.01f || settings.render_low_density ||
                        fluid->get_state(position) != CellType::FLUID) {
                        v3 cell_position = position;
                        cell_position *= v3(fluid->scaling);
                        cell_position += fluid->scaling / 2;

                        float dx = cell_position.x - camera.position.x;
                        float dy = cell_position.y - camera.position.y;
                        float dz = cell_position.z - camera.position.z;
                        float distance_squared = dx * dx + dy * dy + dz * dz;

                        cells.emplace_back(cell_position, distance_squared);
                    }
                }
            }
        }

        std::ranges::sort(cells, std::greater{}, &Cell::distance_squared);

        DrawCubeWiresV(container_center, container_size, GRAY);
        DrawLine3D(v3(0, 0, 0), v3(4, 0, 0), RED);
        DrawLine3D(v3(0, 0, 0), v3(0, 4, 0), GREEN);
        DrawLine3D(v3(0, 0, 0), v3(0, 0, 4), BLUE);
        DrawCubeV(settings.insert_position + v3(fluid->scaling) / 2, v3(fluid->scaling),
                  {255, 0, 0, 100});

        /* Render obstacles */
        BeginBlendMode(BLEND_ALPHA);
        if (settings.show_models) {
            for (auto& obstacle : fluid->obstacles) {
                DrawModel(obstacle->model, obstacle->position * fluid->scaling, fluid->scaling,
                          WHITE);
            }
        }

        /* Render fluid */
        for (const Cell& cell : cells) {
            v3 position = cell.position;

            float density = fluid->get_density(position);
            CellType state = fluid->get_state(position);

            switch (state) {
                case CellType::SOLID:
                    if (settings.show_bounds_solid) DrawCubeV(position, v3(fluid->scaling), BLUE);
                    break;
                case CellType::CUT_CELL:
                    if (settings.show_bounds_cut_cell)
                        DrawCubeV(position, v3(fluid->scaling), GREEN);
                    break;
                case CellType::FLUID:
                    if (settings.show_vel_arrows) {
                        BeginBlendMode(BLEND_SUBTRACT_COLORS);
                        DrawCylinderEx(position, position + (fluid->get_velocity(position) * 100),
                                       density / 100.f, density / 100.f, 10, RED);
                        BeginBlendMode(BLEND_ALPHA);
                    } else {
                        // get color of cube
                        float norm = std::min(density / 100.0f, 1.0f);
                        float hue = (1.0f - norm) * 0.66f * 360.0f;
                        Color c = ColorFromHSV(hue, 1.0f, 1.0f);
                        Color color = {c.r, c.g, c.b, static_cast<uint8_t>(norm * 255)};

                        DrawCubeV(position, v3(fluid->scaling), color);
                    }
                    break;
            }
        }

        EndBlendMode();

        EndMode3D();

        DrawFPS(10, 10);
        if (fluid->should_voxelize) DrawText("Voxelizing...", 10, 30, 20, WHITE);

        bool should_reset = false;
        bool should_rescale = false;
        if (cursor) {
            /* Render UI */
            rlImGuiBegin();
            ImGui::Begin("Fluid Simulation");

            ImGui::Checkbox("Show models", &settings.show_models);
            ImGui::Checkbox("Show velocity with arrows", &settings.show_vel_arrows);
            ImGui::Checkbox("Show bounds SOLID", &settings.show_bounds_solid);
            ImGui::Checkbox("Show bounds CUT_CELL", &settings.show_bounds_cut_cell);
            ImGui::Checkbox("Show cell borders", &settings.show_cell_borders);
            ImGui::Checkbox("Render low density", &settings.render_low_density);

            bool was_cam_free = settings.camera_free;
            ImGui::Checkbox("Camera Free", &settings.camera_free);
            camera.projection = settings.camera_free ? CAMERA_PERSPECTIVE : CAMERA_ORTHOGRAPHIC;
            if (settings.camera_free && !was_cam_free) {
                camera.position = container_size;
                camera.target = container_center;
            }

            ImGui::SliderFloat("Camera FOV", &camera.fovy, 30.0f, 160.0f);

            ImGui::SliderFloat("Diffusion", &fluid->diffusion, 0.0f, 0.0001f);

            int old_container_size = fluid->container_size;
            ImGui::SliderInt("Container size", &fluid->container_size, 1, 64);
            should_reset = old_container_size != fluid->container_size;

            int old_scaling = fluid->scaling;
            ImGui::SliderFloat("Scaling", &fluid->scaling, 0.1f, 10.0f);
            should_rescale = old_scaling != fluid->scaling;

            drag_v3("insert position", settings.insert_position, 1.0f, 1.0f,
                    fluid->container_size);
            drag_v3("insert velocity", settings.insert_velocity, 0.1f, -10.0f, 10.0f);

            for (int i = 0; i < fluid->obstacles.size(); i++) {
                auto& obstacle = fluid->obstacles[i];
                v3 old_pos = obstacle->position;
                drag_v3(TextFormat("Obstacle %s position", obstacle->identifier.c_str()),
                        obstacle->position, 0.1f, 1.0f, fluid->container_size);
                if (old_pos != obstacle->position) fluid->should_voxelize = true;
            }

            if (ImGui::Button("Reset camera")) {
                camera.position = container_size;
                camera.target = container_center;
            }

            ImGui::End();
            ImGui::Render();
            rlImGuiEnd();
        }

        if (fluid->should_voxelize && IsMouseButtonUp(MOUSE_BUTTON_LEFT)) fluid->voxelize_all();

        if (should_reset || should_rescale) {
            container_size = v3(fluid->container_size * fluid->scaling);
            container_center = v3(container_size * 0.5f);
            if (should_reset) fluid->reset();
        }

        EndDrawing();
    }

    delete fluid;
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}
