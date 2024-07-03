package main

import (
	"fmt"
	"strconv"

	rg "github.com/gen2brain/raylib-go/raygui"
	rl "github.com/gen2brain/raylib-go/raylib"

	"github.com/dtasada/paper/src"
)

var particleCount int = 100

func main() {
	fmt.Println("Initializing raylib...")

	/* Init raygui */
	rg.SetStyle(rg.DEFAULT, rg.TEXT_SIZE, 20)

	/* Init raylib */
	rl.InitWindow(1280, 720, "paper")
	rl.SetExitKey(0)
	rl.DisableCursor()
	rl.SetTargetFPS(src.TARGET_FPS)

	/* Raylib flags */
	rl.SetTraceLogLevel(rl.LogWarning)
	rl.SetConfigFlags(rl.FlagMsaa4xHint) // Enable 4x MSAA if available
	rl.SetConfigFlags(rl.FlagWindowResizable)

	/* Shader setup */
	lightShader := rl.LoadShader("./resources/shaders/lighting.vs", "./resources/shaders/lighting.fs")
	*lightShader.Locs = rl.GetShaderLocation(lightShader, "viewPos")
	ambientLoc := rl.GetShaderLocation(lightShader, "ambient")
	shaderValue := []float32{0.1, 0.1, 0.1, 1.0}
	rl.SetShaderValue(lightShader, ambientLoc, shaderValue, rl.ShaderUniformVec4)

	lights := []src.Light{
		src.NewLight(src.LightTypePoint, rl.NewVector3(0, 0, 0), rl.NewVector3(0, -25, 0), rl.Yellow, lightShader),
	}

	/* Logic setup (camera, container and particle arraylist) */
	camera := rl.Camera3D{
		Position:   rl.NewVector3(0, 0, 0),
		Target:     rl.NewVector3(-1, -1, -1),
		Up:         rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		Fovy:       120.0,
		Projection: rl.CameraPerspective,
	}

	container := src.NewContainer(
		rl.NewVector3(0, 0, 0),
		rl.NewVector3(100, 100, 100),
		4,
	)

	particles := []*src.Particle{}

	/* Main loop */
	for !rl.WindowShouldClose() {
		{ /* Pre-render logic here */
			if rl.IsCursorHidden() {
				rl.UpdateCamera(&camera, rl.CameraFree)
			} else if rl.IsMouseButtonPressed(rl.MouseButtonLeft) {
				rl.DisableCursor()
			}

			rl.SetShaderValue(
				lightShader,
				*lightShader.Locs,
				[]float32{camera.Position.X, camera.Position.Y, camera.Position.Z},
				rl.ShaderUniformVec3,
			)

			if rl.IsKeyPressed(rl.KeyEscape) {
				if rl.IsCursorHidden() {
					rl.EnableCursor()
				} else {
					rl.DisableCursor()
				}
			}
		}

		rl.BeginDrawing()
		rl.ClearBackground(rl.Black)

		{ /* 3D Rendering here */
			rl.BeginMode3D(camera)
			for _, particle := range particles {
				particle.Update(&container, &particles)
			}

			for _, light := range lights {
				light.UpdateValues()

				if light.Enabled == 1 {
					rl.DrawSphere(light.Position, 0.2, light.Color)
				} else {
					rl.DrawSphereWires(light.Position, 0.2, 8, 8, rl.Fade(light.Color, 0.3))
				}
			}
			_ = lights

			rl.DrawCubeWires(container.Pos, container.Width, container.Height, container.Length, rl.Red)

			rl.EndMode3D()
		}

		rl.DrawFPS(12, 12+24*0)

		for _, particle := range particles {
			particle.Radius = rg.Slider(
				src.RectTopLeft(30, 80, 240, 20),
				" Particle radius",
				strconv.FormatFloat(float64(particle.Radius), 'f', 2, 64),
				particle.Radius,
				0,
				container.CellSize, // particle can only be as big as one cell
			)

			particleCount = src.Floor(
				rg.Slider(
					src.RectTopLeft(30, 110, 240, 20),
					"Particle count",
					strconv.Itoa(particleCount),
					float32(particleCount),
					0,
					200,
				),
			)
		}

		if len(particles) > particleCount {
			particles = particles[1:]
		} else if len(particles) < particleCount {
			src.CreateParticle(&container, &particles, lightShader)
		}

		rl.EndDrawing()
	}

	{ /* Cleanup */
		rl.UnloadShader(lightShader)
		for _, particle := range particles {
			rl.UnloadModel(particle.Model)
		}
		rl.CloseWindow()
	}
}
