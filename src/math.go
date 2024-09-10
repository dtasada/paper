package src

import (
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"
)

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

func Vector3Invert(vec Vector3) Vector3 {
	return rl.NewVector3(1/vec.X, 1/vec.Y, 1/vec.Z)
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

func Vector3Random(amplitude float32) Vector3 {
	return rl.NewVector3(
		float32(rand.Intn(int(2*amplitude)))-amplitude,
		float32(rand.Intn(int(2*amplitude)))-amplitude,
		float32(rand.Intn(int(2*amplitude)))-amplitude,
	)
}

/* Converts 3D vector to 2D vector by disregarding Z value  */
func Vector3to2(vec rl.Vector3) rl.Vector2 {
	return rl.NewVector2(vec.X, vec.Y)
}

func Vector3MutliplyMatrix3x3(m Matrix3x3, v Vector3) Vector3 {
	return rl.NewVector3(
		m.m11*v.X+m.m12*v.Y+m.m13*v.Z,
		m.m21*v.X+m.m22*v.Y+m.m23*v.Z,
		m.m31*v.X+m.m32*v.Y+m.m33*v.Z,
	)
}

func Matrix3x3Multiply(ma, mb Matrix3x3) Matrix3x3 {
	return Matrix3x3{
		ma.m11*mb.m11 + ma.m12*mb.m21 + ma.m13*mb.m31, ma.m11*mb.m12 + ma.m12*mb.m22 + ma.m13*mb.m32, ma.m11*mb.m13 + ma.m12*mb.m23 + ma.m13*mb.m33,
		ma.m21*mb.m11 + ma.m22*mb.m21 + ma.m23*mb.m31, ma.m21*mb.m12 + ma.m22*mb.m22 + ma.m23*mb.m32, ma.m21*mb.m13 + ma.m22*mb.m23 + ma.m23*mb.m33,
		ma.m31*mb.m11 + ma.m32*mb.m21 + ma.m33*mb.m31, ma.m31*mb.m12 + ma.m32*mb.m22 + ma.m33*mb.m32, ma.m31*mb.m13 + ma.m32*mb.m23 + ma.m33*mb.m33,
	}
}

func Matrix3x3Invert(m Matrix3x3) Matrix3x3 {
	return Matrix3x3{
		1 / m.m11, 1 / m.m12, 1 / m.m13,
		1 / m.m21, 1 / m.m22, 1 / m.m23,
		1 / m.m31, 1 / m.m32, 1 / m.m33,
	}
}

func calcLambda(pa, pb *Particle) float32 {
	d := rl.Vector3Distance(pa.Pos, pb.Pos)
	pointOfContactA := Vector3MultiplyValue(rl.Vector3Subtract(pb.Pos, pa.Pos), pa.Radius/d) // test point of contact
	pointOfContactB := Vector3MultiplyValue(rl.Vector3Subtract(pa.Pos, pb.Pos), pb.Radius/d) // test point of contact

	paQ := Vector3MutliplyMatrix3x3(
		Matrix3x3Multiply(Matrix3x3Invert(pa.Inertia), Matrix3x3Invert(pa.Orientation)),
		rl.Vector3CrossProduct(pointOfContactA, normalOfContact),
	)
	pbQ := Vector3MutliplyMatrix3x3(
		Matrix3x3Multiply(Matrix3x3Invert(pb.Inertia), Matrix3x3Invert(pb.Orientation)),
		rl.Vector3CrossProduct(pointOfContactB, normalOfContact),
	)

	var pbOrientationInverted [3]Vector3
	for i, v3 := range pb.Orientation {
		pbOrientationInverted[i] = Vector3Invert(v3)
	}

	pbQ := rl.NewVector3(
		1/pa.Inertia.X,
		1/pa.Inertia.Y,
		1/pa.Inertia.Z,
	)
	paIQ := rl.Vector3Multiply(pa.Inertia, paQ)
	pbIQ := rl.Vector3Multiply(pb.Inertia, pbQ)

	n := 1
	denumerator := n*(pa.Vel-pb.Vel) + rl.Vector3Multiply(pa.AngVel, paIQ) - rl.Vector3Multiply(pb.AngVel, pbIQ)
	denominator := (1/pa.Mass+1/pb.Mass)*n*n + rl.Vector3Multiply(paIQ*paQ) + rl.Vector3Multiply(pbIQ*pbQ) // Check IQ*Q
	λ := 2 * (denumerator / denominator)
	return λ
}
