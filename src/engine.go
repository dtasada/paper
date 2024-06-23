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

// so many dimesnsions is confusing so i made aliases
type GridCell = []*Particle
type Row = []GridCell
type Plane = []Row

const GRAVITY float32 = 1

/* Returns the inverted color */
func InvertColor(c rl.Color) rl.Color {
	return rl.NewColor(
		255-c.R,
		255-c.G,
		255-c.B,
		255-c.A,
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

/* Returns a Vector3Int from a Vector3 */
func Vector3ToInt(vec Vector3) Vector3Int {
	return Vector3Int{
		int(vec.X),
		int(vec.Y),
		int(vec.Z),
	}
}

func Vector3MultiplyValue(vec Vector3, val float32) Vector3 {
	return rl.NewVector3(vec.X*val, vec.Y*val, vec.Z*val)
}

func Vector3to2(vec rl.Vector3) rl.Vector2 {
	return rl.NewVector2(vec.X, vec.Y)
}

func GetSign(val float32) float32 {
	if val > 0 {
		return 1
	} else if val < 0 {
		return -1
	} else {
		return 0
	}
}

func floorToZero(val float32) int {
	if val > 0 {
		return int(math.Floor(float64(val)))
	} else if val < 0 {
		return -int(math.Floor(float64(-val)))
	} else {
		return 0
	}
}

func Exists[T any](arr []T, index int) bool {
	return (len(arr) > index)
}
