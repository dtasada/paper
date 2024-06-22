package main

import (
	_ "fmt"

	"github.com/gen2brain/raylib-go/raylib"
)

func main() {
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

	particle := NewParticle(
		rl.NewVector3(0, 0, 1),
		rl.Vector3Zero(),
		1,
		rl.SkyBlue,
	)

	bounds := NewBounds(rl.NewVector3(0, 0, 0), 100, 100, 100)

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.Black)

		rl.BeginMode3D(camera)
		rl.UpdateCamera(&camera, rl.CameraFree)

		/* Rendering Logic here */
		particle.Update(&bounds)
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
