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

	container := NewContainer(rl.NewVector3(0, 0, 0), 100, 100, 100)

	particles := []*Particle{}

	// Generate particles
	for i := 1; i <= container.Grid.ZCount; i++ {
		container.Grid.Content = append(container.Grid.Content, [][][]*Particle{})
		axisZ := &container.Grid.Content[len(container.Grid.Content)-1]
		for j := 1; j <= container.Grid.YCount; j++ {
			*axisZ = append(*axisZ, [][]*Particle{})
			axisY := &(*axisZ)[len(*axisZ)-1]
			for k := 1; k <= container.Grid.XCount; k++ {
				if rand.Intn(10) == 1 {
					*axisY = append(*axisY, []*Particle{})
					cell := &(*axisY)[len(*axisY)-1]
					pos := rl.NewVector3(
						rand.Float32()*container.Width-container.Width/2,
						rand.Float32()*container.Height-container.Height/2,
						rand.Float32()*container.Length-container.Length/2,
					)
					p := NewParticle(
						pos,
						rl.Vector3Zero(),
						1,
						rl.SkyBlue,
						i,
						lightShader,
					)
					particles = append(particles, &p) // add to particle arraylist
					fmt.Println("content:", container.Grid.Content)
					*cell = append(*cell, &p) // add to bounds.Grid index
				}
			}
		}
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
			particle.Update(&container, &particles)
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

		rl.DrawCubeWires(container.Pos, container.Width, container.Height, container.Length, rl.Red)

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
