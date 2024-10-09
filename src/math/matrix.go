package math

import "github.com/gen2brain/raylib-go/raylib"

type Matrix = rl.Matrix

func MatrixAdd(matrices ...Matrix) Matrix { return sequential(matrices, rl.MatrixAdd) }

func MatrixSub(matrices ...Matrix) Matrix { return sequential(matrices, rl.MatrixSubtract) }

func MatrixMult(matrices ...Matrix) Matrix { return sequential(matrices, rl.MatrixMultiply) }

func MatrixMultVal(m Matrix, mult float32) Matrix {
	return rl.NewMatrix(
		mult*m.M0, mult*m.M4, mult*m.M8, mult*m.M12,
		mult*m.M1, mult*m.M5, mult*m.M9, mult*m.M13,
		mult*m.M2, mult*m.M6, mult*m.M10, mult*m.M14,
		mult*m.M3, mult*m.M7, mult*m.M11, mult*m.M15,
	)
}

func MatrixInvInertia(inertia Matrix) Matrix {
	return rl.NewMatrix(
		1/inertia.M0, 0, 0, 0,
		0, 1/inertia.M5, 0, 0,
		0, 0, 1/inertia.M10, 0,
		0, 0, 0, 1,
	)
}

func MatrixGlInverse(m Matrix) Matrix {
	var inv Matrix
	inv.M0 = m.M0
	inv.M4 = m.M1
	inv.M8 = m.M2
	inv.M1 = m.M4
	inv.M5 = m.M5
	inv.M9 = m.M6
	inv.M2 = m.M8
	inv.M6 = m.M9
	inv.M10 = m.M10
	inv.M12 -= m.M12*inv.M0 + m.M13*inv.M4 + m.M14*inv.M8
	inv.M13 -= m.M12*inv.M1 + m.M13*inv.M5 + m.M14*inv.M9
	inv.M14 -= m.M12*inv.M2 + m.M13*inv.M6 + m.M14*inv.M10
	inv.M15 = 1
	return inv
}

func MatrixGlDerive(m Matrix) Matrix {
	var r Matrix
	r.M1 = m.M2
	r.M2 = -m.M1
	r.M4 = -m.M2
	r.M6 = m.M0
	r.M8 = m.M1
	r.M9 = -m.M0
	return r
}

func V3ToMatrix(vec V3) Matrix {
	return rl.NewMatrix(
		1, 0, 0, vec.X,
		0, 1, 0, vec.Y,
		0, 0, 1, vec.Z,
		0, 0, 0, 1,
	)
}

func MatrixBitAnd(va, vb Matrix) Matrix {
	var r Matrix
	r.M0 = vb.M2*va.M1 - vb.M1*va.M2
	r.M1 = vb.M0*va.M2 - vb.M2*va.M0
	r.M2 = vb.M1*va.M0 - vb.M0*va.M1
	return r
}

func MatrixBitOr(va, vb Matrix) float32 {
	var sum float32
	for c1 := 0; c1 < 16; c1++ {
		sum += *MatrixIndex(va, c1) * *MatrixIndex(vb, c1)
	}
	return sum
}

func MatrixIndex(m Matrix, index int) *float32 {
	switch index {
	case 0:
		return &m.M0
	case 1:
		return &m.M1
	case 2:
		return &m.M2
	case 3:
		return &m.M3
	case 4:
		return &m.M4
	case 5:
		return &m.M5
	case 6:
		return &m.M6
	case 7:
		return &m.M7
	case 8:
		return &m.M8
	case 9:
		return &m.M9
	case 10:
		return &m.M10
	case 11:
		return &m.M11
	case 12:
		return &m.M12
	case 13:
		return &m.M13
	case 14:
		return &m.M14
	case 15:
		return &m.M15
	default:
		return nil
	}
}
