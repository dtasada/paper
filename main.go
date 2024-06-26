package main

import (
	"fmt"
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"

	"github.com/dtasada/paper/src"
)

var particleCount int = 100

func main() {
	fmt.Println("Initializing raylib...")

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

	/* Generate particles */
	for z := -container.Grid.Planes / 2; z <= container.Grid.Planes/2; z++ {
		zi := z * int(container.CellSize)
		container.Grid.Content[zi] = src.Plane{}
		for y := -container.Grid.Rows / 2; y <= container.Grid.Rows/2; y++ {
			yi := y * int(container.CellSize)
			container.Grid.Content[zi][yi] = src.Row{}
			for x := -container.Grid.Columns / 2; x <= container.Grid.Columns/2; x++ {
				xi := x * int(container.CellSize)
				container.Grid.Content[zi][yi][xi] = src.Cell{}
				if len(particles) < particleCount {
					p := src.NewParticle(
						rl.NewVector3(
							rand.Float32()*container.Width-container.Width/2,
							rand.Float32()*container.Height-container.Height/2,
							rand.Float32()*container.Length-container.Length/2,
						),
						rl.Vector3Zero(),
						container.CellSize,
						src.RandomColor(),
						lightShader,
					)

					particles = append(particles, &p)                                                   // add to particle arraylist
					container.Grid.Content[zi][yi][xi] = append(container.Grid.Content[zi][yi][xi], &p) // add to bounds.Grid index
				}
			}
		}
	}

	/* Main loop */
	for !rl.WindowShouldClose() {
		{ /* Pre-render logic here */
			rl.UpdateCamera(&camera, rl.CameraFree)
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
		rl.DrawText(fmt.Sprint("Camera up", camera.Up), 12, 12+24*1, 22, rl.Green)
		rl.DrawText(fmt.Sprint("Particle count:", particleCount), 12, 12+24*2, 22, rl.Green)

		rl.EndDrawing()
	}

	/* Cleanup */
	{
		rl.UnloadShader(lightShader)
		for _, particle := range particles {
			rl.UnloadModel(particle.Model)
		}
		rl.CloseWindow()
	}
}
