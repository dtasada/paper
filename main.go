package main

import (
	"fmt"
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"
)

func main() {
	fmt.Println("Initializing raylib...")

	rl.InitWindow(1280, 720, "paper")
	rl.SetConfigFlags(rl.FlagMsaa4xHint) // Enable 4x MSAA if available
	rl.SetTargetFPS(60)
	rl.DisableCursor()

	lightShader := rl.LoadShader("./resources/shaders/lighting.vs", "./resources/shaders/lighting.fs")
	*lightShader.Locs = rl.GetShaderLocation(lightShader, "viewPos")
	ambientLoc := rl.GetShaderLocation(lightShader, "ambient")
	shaderValue := []float32{0.1, 0.1, 0.1, 1.0}
	rl.SetShaderValue(lightShader, ambientLoc, shaderValue, rl.ShaderUniformVec4)

	lights := []Light{
		NewLight(LightTypePoint, rl.NewVector3(0, 0, 0), rl.NewVector3(0, -25, 0), rl.Yellow, lightShader),
	}

	var camera rl.Camera3D = rl.Camera3D{
		Position:   rl.NewVector3(-10, -25, 0),
		Target:     rl.NewVector3(0, 0, 1),
		Up:         rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		Fovy:       100.0,
		Projection: rl.CameraPerspective,
	}

	bounds := NewBounds(rl.NewVector3(0, 0, 0), 100, 100, 100)
	// grid := Grid{100, 100, 100}

	particles := []*Particle{}
	particleCount := 1024

	for i := 0; i < particleCount; i++ {
		particles = append(particles, NewParticle(
			rl.NewVector3(
				float32(rand.Intn(int(bounds.Width))-int(bounds.Width)/2),
				float32(rand.Intn(int(bounds.Height))-int(bounds.Height/2)),
				float32(rand.Intn(int(bounds.Length))-int(bounds.Length/2)),
			),
			rl.Vector3Zero(),
			1,
			rl.SkyBlue,
			i,
			lightShader,
		))
	}

	for !rl.WindowShouldClose() {
		rl.UpdateCamera(&camera, rl.CameraFree)
		rl.SetShaderValue(
			lightShader,
			*lightShader.Locs,
			[]float32{camera.Position.X, camera.Position.Y, camera.Position.Z},
			rl.ShaderUniformVec3,
		)

		rl.BeginDrawing()
		rl.ClearBackground(rl.Black)

		rl.BeginMode3D(camera)

		/* Rendering Logic here */
		for _, particle := range particles {
			particle.Update(&bounds, &particles)
			fmt.Println("i:", particle.Id, "pos:", particle.Pos)
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

		rl.DrawCubeWires(bounds.Pos, bounds.Width, bounds.Height, bounds.Length, rl.Red)

		rl.EndMode3D()

		rl.DrawFPS(12, 12)

		rl.EndDrawing()
	}

	rl.UnloadShader(lightShader)
	for _, particle := range particles {
		rl.UnloadModel(particle.Model)
	}
	rl.CloseWindow()
}
