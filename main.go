package main

import (
	_ "github.com/gen2brain/raylib-go/raylib"
	rl "github.com/gen2brain/raylib-go/raylib"
)

func main() {
	rl.InitWindow(1280, 720, "paper")
	rl.SetTargetFPS(60)

	bounds := rl.NewRectangle(
		20,
		20,
		float32(rl.GetScreenWidth()-40),
		float32(rl.GetScreenHeight()-40),
	)
	grid := NewGrid()
	particles := []Particle{}

	/* Generate particles */
	for i := 1; float32(i) <= grid.Height; i++ {
		grid.Content = append(grid.Content, []GridCell{})
		for j := 1; float32(j) <= grid.Width; j++ {
			grid.Content[i-1] = append(grid.Content[i-1], GridCell{})
			newParticle := NewParticle(
				rl.NewVector2(bounds.X+float32(j)*50, bounds.Y+float32(i)*50),
				rl.Vector2Zero(),
				20.0,
			)

			particles = append(particles, newParticle)
			grid.Content[i-1][j-1] = append(grid.Content[i-1][j-1], &newParticle)
		}
	}

	for !rl.WindowShouldClose() {
		rl.BeginDrawing()
		rl.ClearBackground(rl.Black)

		for _, particle := range particles {
			particle.Update(&bounds, &grid)
		}

		grid.FindCollisions()

		rl.DrawFPS(12, 12)
		rl.DrawRectangleLinesEx(bounds, 1, rl.Orange)
	}
}
