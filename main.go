package main

import (
	"fmt"
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"

	"github.com/dtasada/paper/src"
)

func main() {
	fmt.Println("Initializing raylib...")

	/* Set raylib options */
	rl.SetTraceLogLevel(rl.LogWarning)
	rl.SetConfigFlags(rl.FlagMsaa4xHint) // Enable 4x MSAA if available
	rl.SetConfigFlags(rl.FlagWindowResizable)
	rl.SetTargetFPS(60)
	rl.SetExitKey(0)

	/* Init raylb window */
	rl.InitWindow(1280, 720, "paper")
	rl.DisableCursor()

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
		Position:   rl.NewVector3(-10, -25, 0),
		Target:     rl.NewVector3(0, 0, 1),
		Up:         rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		Fovy:       100.0,
		Projection: rl.CameraPerspective,
	}

	container := src.NewContainer(
		rl.NewVector3(0, 0, 0),
		rl.NewVector3(100, 100, 100),
		rl.NewVector3(100, 100, 100),
	)

	particles := []*src.Particle{}

	/* Generate particles */
	for z := 0; z <= container.Grid.Planes; z++ {
		container.Grid.Content = append(container.Grid.Content, src.Plane{})
		plane := &container.Grid.Content[z]
		for y := 0; y <= container.Grid.Rows; y++ {
			*plane = append(*plane, src.Row{})
			row := &(*plane)[y]
			for x := 0; x <= container.Grid.Columns; x++ {
				*row = append(*row, src.GridCell{})
				cell := &(*row)[x]
				// if rand.Intn(100000) == 1 {
				if len(particles) == 0 {
					pos := rl.NewVector3(
						rand.Float32()*container.Width-container.Width/2,
						rand.Float32()*container.Height-container.Height/2,
						rand.Float32()*container.Length-container.Length/2,
					)
					p := src.NewParticle(
						pos,
						rl.Vector3Zero(),
						1,
						rl.SkyBlue,
						lightShader,
					)

					particles = append(particles, &p) // add to particle arraylist
					*cell = append(*cell, &p)         // add to bounds.Grid index
				}
			}
		}
	}
	// container.Grid.PrintContent()

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

		rl.DrawFPS(12, 12)

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
