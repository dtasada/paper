package math

import (
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"
)

type V2 = rl.Vector2
type V3 = rl.Vector3
type V3Int struct {
	X, Y, Z int
}

/* Returns a Vector3 from integers (used to save on type assertions) */
func NewV3FromInt(x, y, z int) V3 {
	return rl.NewVector3(
		float32(x),
		float32(y),
		float32(z),
	)
}

func V3IntToV3(vec V3Int) V3 {
	return rl.NewVector3(
		float32(vec.X),
		float32(vec.Y),
		float32(vec.Z),
	)
}

/* Returns a Vector3Int from a Vector3 */
func V3ToV3Int(vec V3) V3Int {
	return V3Int{
		int(vec.X),
		int(vec.Y),
		int(vec.Z),
	}
}

func V3Invert(vec V3) V3 {
	return rl.NewVector3(1/vec.X, 1/vec.Y, 1/vec.Z)
}

func V3DivVal(vec V3, div float32) V3 {
	return rl.NewVector3(vec.X/div, vec.Y/div, vec.Z/div)
}

func V3Add(vectors ...V3) V3 { return sequential(vectors, rl.Vector3Add) }

func V3Sub(vectors ...V3) V3 { return sequential(vectors, rl.Vector3Subtract) }

func V3Mult(vectors ...V3) V3 { return sequential(vectors, rl.Vector3Multiply) }

func V3MultVal(vec V3, mult float32) V3 {
	return rl.NewVector3(vec.X*mult, vec.Y*mult, vec.Z*mult)
}

func V3IntMultValue(vec V3Int, mult int) V3Int {
	return V3Int{vec.X * mult, vec.Y * mult, vec.Z * mult}
}

func V3Random(amplitude float32) V3 {
	return rl.NewVector3(
		float32(rand.Intn(int(2*amplitude)))-amplitude,
		float32(rand.Intn(int(2*amplitude)))-amplitude,
		float32(rand.Intn(int(2*amplitude)))-amplitude,
	)
}

/* Converts 3D vector to 2D vector by disregarding Z value  */
func V3ToV2(vec rl.Vector3) rl.Vector2 {
	return rl.NewVector2(vec.X, vec.Y)
}
