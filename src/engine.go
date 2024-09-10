/* package src: base engine functions structs and constants */
package src

import (
	"math"

	"github.com/gen2brain/raylib-go/raylib"
)

type f32 = float32
type f64 = float64

type Vector2 = rl.Vector2
type Vector3 = rl.Vector3
type Vector3Int struct {
	X, Y, Z int
}

type Matrix3x3 struct {
	m11, m12, m13 float32
	m21, m22, m23 float32
	m31, m32, m33 float32
}

// so many dimensions is confusing so i made aliases
type Cell = []*Particle
type Row = map[int]Cell
type Plane = map[int]Row

var GravityMultiplier float32 = 250
var Gravity float32 = 1 / GravityMultiplier // gravity in m/s^2
var TargetFPS int32 = 60
var Sensitivity float32 = 0.0035
var MovementSpeed float32 = 1.0

var ShowCollisionGrid bool = false
var ShowCollisionLines bool = false
var ShowParticleCells bool = false
var ShowCellModels bool = true

var Caskaydia rl.Font

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
	rl.DrawTextEx(Caskaydia, text, rl.NewVector2(x, y), fontSize, 1, tint)
}

/* Returns sign of value. -1 if negative, 0 if 0, and 1 if positive */
func GetSign(val float32) float32 {
	if val > 0 {
		return 1
	} else if val < 0 {
		return -1
	} else {
		return 0
	}
}

/* math.Floor but without all the Go float bs */
func Floor(val float32) int {
	return int(math.Floor(float64(val)))
}

/* Floors value, but rounds negative numbers up */
func FloorToZero(val float32) int {
	if val > 0 {
		return Floor(val)
	} else if val < 0 {
		return -Floor(-val)
	} else {
		return 0
	}
}

/* Rounds value the nearest cell */
func RoundToCellSize(val float32, cellSize float32) int {
	r := float32(math.Mod(float64(val), float64(cellSize)))
	if r+r >= cellSize {
		return FloorToZero(val + cellSize - r)
	} else {
		return FloorToZero(val - r)
	}
}
