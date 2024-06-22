package main

import (
	"fmt"
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"
)

func main() {
	fmt.Println("Initializing raylib...")
	rl.InitWindow(1280, 720, "paper")
	rl.SetTargetFPS(60)
	rl.DisableCursor()

	var camera rl.Camera3D = rl.Camera3D{
		Position:   rl.NewVector3(0, -49, 0),
		Target:     rl.NewVector3(0, 0, 1),
		Up:         rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		Fovy:       100.0,
		Projection: rl.CameraPerspective,
	}

	bounds := NewBounds(rl.NewVector3(0, 0, 0), 100, 100, 100)

	// grid := Grid{100, 100, 100}

	particles := []*Particle{}
	particleCount := 24

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
			0,
		))
	}

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.Black)

		rl.BeginMode3D(camera)
		rl.UpdateCamera(&camera, rl.CameraFree)

		/* Rendering Logic here */
		for _, particle := range particles {
			particle.Update(&bounds)
			fmt.Println("i:", particle.Id, "pos:", particle.Pos)
		}

		rl.DrawCubeWires(bounds.Pos, bounds.Width, bounds.Height, bounds.Length, rl.Red)
		/* rl.DrawPlane(
			rl.NewVector3(bounds.Pos.X, bounds.Pos.Y-bounds.Height/2, bounds.Pos.Z),
			rl.NewVector2(bounds.Width, bounds.Length),
			rl.Green,
		) */

		rl.EndMode3D()

		rl.DrawFPS(12, 12)

		rl.EndDrawing()
	}

	rl.CloseWindow()
}
