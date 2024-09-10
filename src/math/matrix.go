package math

import "github.com/gen2brain/raylib-go/raylib"

type Matrix struct {
	M11, M12, M13 float32
	M21, M22, M23 float32
	M31, M32, M33 float32
}

func NewMatrix(
	M11, M12, M13 float32,
	M21, M22, M23 float32,
	M31, M32, M33 float32,
) Matrix {
	return Matrix{
		M11, M12, M13,
		M21, M22, M23,
		M31, M32, M33,
	}
}

func MatrixAdd(matrices ...Matrix) Matrix {
	return sequential(matrices, func(ma, mb Matrix) Matrix {
		return Matrix{
			ma.M11 + mb.M11, ma.M12 + mb.M12, ma.M13 + mb.M13,
			ma.M21 + mb.M21, ma.M22 + mb.M22, ma.M23 + mb.M23,
			ma.M31 + mb.M31, ma.M32 + mb.M32, ma.M33 + mb.M33,
		}
	})
}

func MatrixMult(matrices ...Matrix) Matrix {
	return sequential(matrices, func(ma, mb Matrix) Matrix {
		return Matrix{
			ma.M11*mb.M11 + ma.M12*mb.M21 + ma.M13*mb.M31, ma.M11*mb.M12 + ma.M12*mb.M22 + ma.M13*mb.M32, ma.M11*mb.M13 + ma.M12*mb.M23 + ma.M13*mb.M33,
			ma.M21*mb.M11 + ma.M22*mb.M21 + ma.M23*mb.M31, ma.M21*mb.M12 + ma.M22*mb.M22 + ma.M23*mb.M32, ma.M21*mb.M13 + ma.M22*mb.M23 + ma.M23*mb.M33,
			ma.M31*mb.M11 + ma.M32*mb.M21 + ma.M33*mb.M31, ma.M31*mb.M12 + ma.M32*mb.M22 + ma.M33*mb.M32, ma.M31*mb.M13 + ma.M32*mb.M23 + ma.M33*mb.M33,
		}
	})
}

func MatrixMultVal(m Matrix, mult float32) Matrix {
	return Matrix{
		m.M11*mult + m.M12*mult + m.M13*mult, m.M11*mult + m.M12*mult + m.M13*mult, m.M11*mult + m.M12*mult + m.M13*mult,
		m.M21*mult + m.M22*mult + m.M23*mult, m.M21*mult + m.M22*mult + m.M23*mult, m.M21*mult + m.M22*mult + m.M23*mult,
		m.M31*mult + m.M32*mult + m.M33*mult, m.M31*mult + m.M32*mult + m.M33*mult, m.M31*mult + m.M32*mult + m.M33*mult,
	}
}

func MatrixInvert(m Matrix) Matrix {
	return Matrix{
		1 / m.M11, 1 / m.M12, 1 / m.M13,
		1 / m.M21, 1 / m.M22, 1 / m.M23,
		1 / m.M31, 1 / m.M32, 1 / m.M33,
	}
}

func V3MultMatrix(v V3, m Matrix) V3 {
	return rl.NewVector3(
		m.M11*v.X+m.M12*v.Y+m.M13*v.Z,
		m.M21*v.X+m.M22*v.Y+m.M23*v.Z,
		m.M31*v.X+m.M32*v.Y+m.M33*v.Z,
	)
}
