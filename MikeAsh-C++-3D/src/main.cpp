#include <bits/stdc++.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <cstdio>

#include "../include/Fluid.hpp"
#include "../include/Light.hpp"
#include "../include/Profiler.hpp"

#define FPS 60.0f
#define VECTOROF(x) \
    (v3) { (float)x, (float)x, (float)x }

int main(int argc, char* argv[]) {
    srand(time(nullptr));

    InitWindow(1280, 720, "Fluid Grid");
    SetTargetFPS(FPS);
    SetExitKey(0);

    bool cursor = false;
    DisableCursor();
    // HideCursor();

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    float diffusion = 0;         // Diffusion constant
    float dt = 0.1;              // Timestep
    float viscosity = 0.000001;  // Viscosity constant

    Camera3D camera = {
        .position = v3(10, 10, 10),
        .target = v3(-100, -100, -100),
        .up = {0.0, 1.0, 0.0},
        .fovy = 90.0f,
        .projection = CAMERA_PERSPECTIVE,
    };

    Fluid fluid(24, 1, diffusion, viscosity, dt);
    v3 containerSize = VECTOROF(fluid.ContainerSize() * fluid.FluidSize());
    v3 containerCenter = containerSize * 0.5f;
    v3 render_index, amount = v3();

    /* Model model =
        LoadModelFromMesh(GenMeshCube(fluid.FluidSize(), fluid.FluidSize(), fluid.FluidSize()));
    Shader lightShader =
        LoadShader("resources/shaders/vertex.glsl", "resources/shaders/fragment.glsl");
    *lightShader.locs = GetShaderLocation(lightShader, "viewPos");
    int ambientLoc = GetShaderLocation(lightShader, "ambient");
    float* shaderValue = new float[4]{0.1, 0.1, 0.1, 1.0};
    SetShaderValue(lightShader, ambientLoc, shaderValue, SHADER_UNIFORM_IVEC4);
    model.materials->shader = lightShader;

    Light light(DIRECTIONAL, camera.position, camera.target, YELLOW, 1, lightShader); */

    while (!WindowShouldClose()) {
        // SetShaderValue(lightShader, *lightShader.locs, &camera.position, SHADER_UNIFORM_VEC3);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            amount.x = amount.y = amount.z = rand() % 11 - 5;
            // float x = constrain(camera.position.x, 0, fluid.ContainerSize());
            // float y = constrain(camera.position.y, 0, fluid.ContainerSize());
            // float z = constrain(camera.position.z, 0, fluid.ContainerSize());

            float x = 1, y = 1, z = 1;
            fluid.addDensity({x, y, z}, 100);
            fluid.addVelocity({x, y, z}, amount);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            amount.x = amount.y = amount.z = rand() % 11 - 5;

            float x = 1, y = 1, z = 1;
            fluid.addDensity({x, y, z}, 10000);
            fluid.addVelocity({x, y, z}, amount);
        }

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

        if (!cursor) {
            UpdateCamera(&camera, CAMERA_FREE);
            // light.SetPosition(camera.position);
            // light.SetTarget(camera.target);
        }

        fluid.step();

        BeginDrawing();
        ClearBackground(BLUE);

        BeginMode3D(camera);
        // light.update();

        DrawCubeWiresV(containerCenter, containerSize, RED);

        for (float z = 0; z < fluid.ContainerSize(); z++) {
            for (float y = 0; y < fluid.ContainerSize(); y++) {
                for (float x = 0; x < fluid.ContainerSize(); x++) {
                    float density = fluid.Density({x, y, z});
                    if (density <= 10e-7) continue;
                    render_index = {x, y, z};
                    render_index = render_index * VECTOROF(fluid.FluidSize());
                    render_index = render_index + (fluid.FluidSize() / 2);
                    BeginBlendMode(BLEND_ALPHA);
                    DrawCubeV(render_index, VECTOROF(fluid.FluidSize()),
                              {255, 70, 0, (uint8_t)density});
                    EndBlendMode();
                }
            }
        }

        EndMode3D();

        DrawFPS(10, 10);
        DrawText(std::string("Position: ").append(v3(camera.position).to_string()).c_str(), 10, 30,
                 20, RAYWHITE);

        EndDrawing();
    }

    CloseWindow();
    Profiler::printResults();
    return 0;
}
