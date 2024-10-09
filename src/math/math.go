package math

import "math"

/*
 * Abstract function that generates sequential arithmetic functions,
 * where passing it something like f = rl.Vector3Add calls f(list[n], list[n-1])
 */
func sequential[T V3 | Matrix](list []T, f func(T, T) T) T {
	var r T
	for i, seq := range list {
		if i == 0 {
			r = seq
		} else {
			r = f(r, seq)
		}
	}
	return r
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

/* math but without all the go/math float64 bs */
func Sin(val float32) float32                 { return float32(math.Sin(float64(val))) }
func Cos(val float32) float32                 { return float32(math.Cos(float64(val))) }
func Floor(val float32) int                   { return int(math.Floor(float64(val))) }
func Pow[T, U int | float32](base T, pow U) T { return T(math.Pow(float64(base), float64(pow))) }

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
