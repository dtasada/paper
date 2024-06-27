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

// so many dimensions is confusing so i made aliases
type Cell = []*Particle
type Row = map[int]Cell
type Plane = map[int]Row

const GRAVITY float32 = 9.81 / 400
const TARGET_FPS int32 = 60

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

/* Returns a Vector3 from integers (used to save on type assertions) */
func NewVector3FromInt(x, y, z int) Vector3 {
	return rl.NewVector3(
		float32(x),
		float32(y),
		float32(z),
	)
}

func Vector3IntToVector3(vec Vector3Int) Vector3 {
	return rl.NewVector3(
		float32(vec.X),
		float32(vec.Y),
		float32(vec.Z),
	)
}

/* Returns a Vector3Int from a Vector3 */
func Vector3ToInt(vec Vector3) Vector3Int {
	return Vector3Int{
		int(vec.X),
		int(vec.Y),
		int(vec.Z),
	}
}

func Vector3DivideValue(vec Vector3, div float32) Vector3 {
	return rl.NewVector3(vec.X/div, vec.Y/div, vec.Z/div)
}

func Vector3MultiplyValue(vec Vector3, mult float32) Vector3 {
	return rl.NewVector3(vec.X*mult, vec.Y*mult, vec.Z*mult)
}

func Vector3IntMultiplyValue(vec Vector3Int, mult int) Vector3Int {
	return Vector3Int{vec.X * mult, vec.Y * mult, vec.Z * mult}
}

/* Converts 3D vector to 2D vector by disregarding Z value  */
func Vector3to2(vec rl.Vector3) rl.Vector2 {
	return rl.NewVector2(vec.X, vec.Y)
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

/* Floors value, but rounds negative numbers up */
func FloorToZero(val float32) int {
	if val > 0 {
		return int(math.Floor(float64(val)))
	} else if val < 0 {
		return -int(math.Floor(float64(-val)))
	} else {
		return 0
	}
}

/* Rounds value to tens */
func FloorTens(val float32) int {
	return int(FloorToZero(val/10)) * 10
}
