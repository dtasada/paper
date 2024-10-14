/* package engine: base engine functions structs and constants */
package engine

import (
	"math"

	m "github.com/dtasada/paper/engine/math"
	"github.com/gen2brain/raylib-go/raylib"
)

type ( // so many dimensions is confusing so i made aliases
	Cell  = []*Particle
	Row   = map[int]Cell
	Plane = map[int]Row
)

var (
	GravityMultiplier float32 = 1e4
	Gravity           float32 = 1 / GravityMultiplier // gravity in m/s^2
	Sensitivity       float32 = 0.0035
	MovementSpeed     float32 = 1.0
	ForceMult         float32 = 1
	TargetFPS         int32   = 60

	Font rl.Font
)

var (
	ContainParticles   bool = true
	ShowCellModels     bool = true
	ShowCollisionGrid  bool = false
	ShowCollisionLines bool = false
	ShowContainerWalls bool = true
	ShowParticleCells  bool = false
)

/* Returns the inverted color */
func InvertColor(c rl.Color) rl.Color {
	return rl.NewColor(
		255-c.R,
		255-c.G,
		255-c.B,
		255-c.A,
	)
}

/* Returns color with random RGBA values */
func RandomColor() rl.Color {
	return rl.NewColor(
		uint8(rl.GetRandomValue(0, 255)),
		uint8(rl.GetRandomValue(0, 255)),
		uint8(rl.GetRandomValue(0, 255)),
		uint8(rl.GetRandomValue(0, 255)),
	)
}

/* Returns a rectangle width top-left at x and y */
func RectTopLeft(x, y, width, height float32) rl.Rectangle {
	return rl.NewRectangle(
		x+width/2,
		y+height/2,
		width,
		height,
	)
}

func DrawText(text string, x, y float32, fontSize float32, tint rl.Color) {
	rl.DrawTextEx(Font, text, rl.NewVector2(x, y), fontSize, 1, tint)
}

/* Rounds value the nearest cell */
func RoundToCellSize(val float32, cellSize float32) int {
	r := float32(math.Mod(float64(val), float64(cellSize)))
	if r+r >= cellSize {
		return m.FloorToZero(val + cellSize - r)
	} else {
		return m.FloorToZero(val - r)
	}
}
