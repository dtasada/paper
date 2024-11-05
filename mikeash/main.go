package main

import (
	"math"

	"github.com/gen2brain/raylib-go/raylib"
)

const SENSITIVITY = 0.0035
const MOVEMENT_SPEED = 1.0
const CUBE_SIZE = 64
const SCALE = 10

func IX(x, y, z, N int) int {
	x = min(max(x, 0), N-1)
	y = min(max(y, 0), N-1)
	z = min(max(z, 0), N-1)
	return x + y*N + z*N*N
}

func IXv(v rl.Vector3, N int) int {
	xi := min(max(int(v.X), 0), N-1)
	yi := min(max(int(v.Y), 0), N-1)
	zi := min(max(int(v.Z), 0), N-1)
	return xi + int(yi)*N + int(zi)*N*N
}

func SetBnd(b int, x []float32, N int) {
	for j := 1; j < N-1; j++ {
		for i := 1; i < N-1; i++ {
			if b == 3 {
				x[IX(i, j, 0, N)] = -x[IX(i, j, 1, N)]
				x[IX(i, j, N-1, N)] = -x[IX(i, j, N-2, N)]
			} else {
				x[IX(i, j, 0, N)] = x[IX(i, j, 1, N)]
				x[IX(i, j, N-1, N)] = x[IX(i, j, N-2, N)]
			}
		}
	}

	for k := 1; k < N-1; k++ {
		for i := 1; i < N-1; i++ {
			if b == 2 {
				x[IX(i, 0, k, N)] = -x[IX(i, 1, k, N)]
				x[IX(i, N-1, k, N)] = -x[IX(i, N-2, k, N)]
			} else {
				x[IX(i, 0, k, N)] = x[IX(i, 1, k, N)]
				x[IX(i, N-1, k, N)] = x[IX(i, N-2, k, N)]
			}
		}
	}

	for k := 1; k < N-1; k++ {
		for j := 1; j < N-1; j++ {
			if b == 1 {
				x[IX(0, j, k, N)] = -x[IX(1, j, k, N)]
				x[IX(N-1, j, k, N)] = -x[IX(N-2, j, k, N)]
			} else {
				x[IX(0, j, k, N)] = x[IX(1, j, k, N)]
				x[IX(N-1, j, k, N)] = x[IX(N-2, j, k, N)]
			}
		}
	}

	x[IX(0, 0, 0, N)] = 0.33 * (x[IX(1, 0, 0, N)] + x[IX(0, 1, 0, N)] + x[IX(0, 0, 1, N)])
	x[IX(0, N-1, 0, N)] = 0.33 * (x[IX(1, N-1, 0, N)] + x[IX(0, N-2, 0, N)] + x[IX(0, N-1, 1, N)])
	x[IX(0, 0, N-1, N)] = 0.33 * (x[IX(1, 0, N-1, N)] + x[IX(0, 1, N-1, N)] + x[IX(0, 0, N, N)])
	x[IX(0, N-1, N-1, N)] = 0.33 * (x[IX(1, N-1, N-1, N)] + x[IX(0, N-2, N-1, N)] + x[IX(0, N-1, N-2, N)])
	x[IX(N-1, 0, 0, N)] = 0.33 * (x[IX(N-2, 0, 0, N)] + x[IX(N-1, 1, 0, N)] + x[IX(N-1, 0, 1, N)])
	x[IX(N-1, N-1, 0, N)] = 0.33 * (x[IX(N-2, N-1, 0, N)] + x[IX(N-1, N-2, 0, N)] + x[IX(N-1, N-1, 1, N)])
	x[IX(N-1, 0, N-1, N)] = 0.33 * (x[IX(N-2, 0, N-1, N)] + x[IX(N-1, 1, N-1, N)] + x[IX(N-1, 0, N-2, N)])
	x[IX(N-1, N-1, N-1, N)] = 0.33 * (x[IX(N-2, N-1, N-1, N)] + x[IX(N-1, N-2, N-1, N)] + x[IX(N-1, N-1, N-2, N)])
}

func LinSolve(b int, x, x0 []float32, a, c float32, iter, N int) {
	cRecip := 1.0 / c
	for k := 0; k < iter; k++ {
		for m := 1; m < N-1; m++ {
			for j := 1; j < N-1; j++ {
				for i := 1; i < N-1; i++ {
					x[IX(i, j, m, N)] = (x0[IX(i, j, m, N)] +
						a*(x[IX(i+1, j, m, N)]+
							x[IX(i-1, j, m, N)]+
							x[IX(i, j+1, m, N)]+
							x[IX(i, j-1, m, N)]+
							x[IX(i, j, m+1, N)]+
							x[IX(i, j, m-1, N)])) * cRecip
				}
			}
		}

		SetBnd(b, x, N)
	}
}

func Diffuse(b int, x, x0 []float32, diff, dt float32, iter, N int) {
	// a := dt * diff * float32((N-2)*(N-2))
	a := dt * diff * float32(math.Pow(SCALE, -2))
	LinSolve(b, x, x0, a, 1+6*a, iter, N)
}

func Project(velocX, velocY, velocZ, p, div []float32, iter, N int) {
	for k := 1; k < N-1; k++ {
		for j := 1; j < N-1; j++ {
			for i := 1; i < N-1; i++ {
				div[IX(i, j, k, N)] =
					-0.5 *
						(velocX[IX(i+1, j, k, N)] - velocX[IX(i-1, j, k, N)] +
							velocY[IX(i, j+1, k, N)] - velocY[IX(i, j-1, k, N)] +
							velocZ[IX(i, j, k+1, N)] - velocZ[IX(i, j, k-1, N)]) /
						float32(N)
				p[IX(i, j, k, N)] = 0
			}
		}
	}
	SetBnd(0, div, N)
	SetBnd(0, p, N)
	LinSolve(0, p, div, 1, 6, iter, N)

	for k := 1; k < N-1; k++ {
		for j := 1; j < N-1; j++ {
			for i := 1; i < N-1; i++ {
				velocX[IX(i, j, k, N)] -= 0.5 * (p[IX(i+1, j, k, N)] - p[IX(i-1, j, k, N)]) * float32(N)
				velocY[IX(i, j, k, N)] -= 0.5 * (p[IX(i, j+1, k, N)] - p[IX(i, j-1, k, N)]) * float32(N)
				velocZ[IX(i, j, k, N)] -= 0.5 * (p[IX(i, j, k+1, N)] - p[IX(i, j, k-1, N)]) * float32(N)
			}
		}
	}
	SetBnd(1, velocX, N)
	SetBnd(2, velocY, N)
	SetBnd(3, velocZ, N)
}

func floorf(x float32) float32 {
	return float32(math.Floor(float64(x)))
}

func Advect(b int, d, d0, velocX, velocY, velocZ []float32, dt float32, N int) {
	var i0, i1, j0, j1, k0, k1 float32

	dtx := dt * float32(N-2)
	dty := dt * float32(N-2)
	dtz := dt * float32(N-2)

	var s0, s1, t0, t1, u0, u1 float32
	var tmp1, tmp2, tmp3, x, y, z float32

	Nfloat := float32(N)
	var ifloat, jfloat, kfloat float32
	var i, j, k int

	for k, kfloat = 1, 1; k < N-1; k, kfloat = k+1, kfloat+1 {
		for j, jfloat = 1, 1; j < N-1; j, jfloat = j+1, jfloat+1 {
			for i, ifloat = 1, 1; i < N-1; i, ifloat = i+1, ifloat+1 {
				tmp1 = dtx * velocX[IX(i, j, k, N)]
				tmp2 = dty * velocY[IX(i, j, k, N)]
				tmp3 = dtz * velocZ[IX(i, j, k, N)]
				x = ifloat - tmp1
				y = jfloat - tmp2
				z = kfloat - tmp3

				if x < 0.5 {
					x = 0.5
				}
				if x > Nfloat+0.5 {
					x = Nfloat + 0.5
				}
				i0 = floorf(x)
				i1 = i0 + 1.0
				if y < 0.5 {
					y = 0.5
				}
				if y > Nfloat+0.5 {
					y = Nfloat + 0.5
				}
				j0 = floorf(y)
				j1 = j0 + 1.0
				if z < 0.5 {
					z = 0.5
				}
				if z > Nfloat+0.5 {
					z = Nfloat + 0.5
				}
				k0 = floorf(z)
				k1 = k0 + 1.0

				s1 = x - i0
				s0 = 1.0 - s1
				t1 = y - j0
				t0 = 1.0 - t1
				u1 = z - k0
				u0 = 1.0 - u1

				i0i := int(i0)
				i1i := int(i1)
				j0i := int(j0)
				j1i := int(j1)
				k0i := int(k0)
				k1i := int(k1)

				d[IX(i, j, k, N)] = s0*(t0*(u0*d0[IX(i0i, j0i, k0i, N)]+
					u1*d0[IX(i0i, j0i, k1i, N)])+
					(t1*(u0*d0[IX(i0i, j1i, k0i, N)]+
						u1*d0[IX(i0i, j1i, k1i, N)]))) +
					s1*(t0*(u0*d0[IX(i1i, j0i, k0i, N)]+
						u1*d0[IX(i1i, j0i, k1i, N)])+
						(t1*(u0*d0[IX(i1i, j1i, k0i, N)]+
							u1*d0[IX(i1i, j1i, k1i, N)])))
			}
		}
	}
	SetBnd(b, d, N)
}

type FluidCube struct {
	size int
	dt   float32
	diff float32
	visc float32

	s       []float32
	density []float32

	Vx []float32
	Vy []float32
	Vz []float32

	Vx0 []float32
	Vy0 []float32
	Vz0 []float32
}

func NewFluidCube(size, diffusion, viscosity int, dt float32) *FluidCube {
	return &FluidCube{
		size: size,
		dt:   dt,
		diff: float32(diffusion),
		visc: float32(viscosity),

		s:       []float32{},
		density: []float32{},
		Vx:      []float32{},
		Vy:      []float32{},
		Vz:      []float32{},
		Vx0:     []float32{},
		Vy0:     []float32{},
		Vz0:     []float32{},
	}
}

func (cube *FluidCube) AddDensity(pos rl.Vector3, amount float32) {
	cube.density[IXv(pos, cube.size)] += amount
}

func (cube *FluidCube) AddVelocity(pos, amount rl.Vector3) {
	index := IXv(pos, cube.size)

	cube.Vx[index] += amount.X
	cube.Vy[index] += amount.Y
	cube.Vz[index] += amount.Z
}

func (cube *FluidCube) Step() {
	Diffuse(1, cube.Vx0, cube.Vx, cube.visc, cube.dt, 4, cube.size)
	Diffuse(2, cube.Vy0, cube.Vy, cube.visc, cube.dt, 4, cube.size)
	Diffuse(3, cube.Vz0, cube.Vz, cube.visc, cube.dt, 4, cube.size)

	Project(cube.Vx0, cube.Vy0, cube.Vz0, cube.Vx, cube.Vy, 4, cube.size)

	Advect(1, cube.Vx, cube.Vx0, cube.Vx0, cube.Vy0, cube.Vz0, cube.dt, cube.size)
	Advect(2, cube.Vy, cube.Vy0, cube.Vx0, cube.Vy0, cube.Vz0, cube.dt, cube.size)
	Advect(3, cube.Vz, cube.Vz0, cube.Vx0, cube.Vy0, cube.Vz0, cube.dt, cube.size)

	Project(cube.Vx, cube.Vy, cube.Vz, cube.Vx0, cube.Vy0, 4, cube.size)

	Diffuse(0, cube.s, cube.density, cube.diff, cube.dt, 4, cube.size)
	Advect(0, cube.density, cube.s, cube.Vx, cube.Vy, cube.Vz, cube.dt, cube.size)
}

func (cube *FluidCube) RenderD(camera *rl.Camera3D) {
	for i := 0; i < cube.size; i++ {
		for j := 0; j < cube.size; j++ {
			for k := 0; k < cube.size; k++ {
				d := cube.density[IXv(camera.Position, cube.size)]
				rl.DrawCube(
					rl.NewVector3(float32(i), float32(j), float32(k)),
					float32(cube.size),
					float32(cube.size),
					float32(cube.size),
					rl.NewColor(uint8(d+50)%255, 200, uint8(d), 255),
				)
			}
		}
	}
}

func handleMovement(camera *rl.Camera3D) {
	var MovementSpeed float32 = 1.0
	var Sensitivity float32 = 0.0035

	mouseDelta := rl.GetMouseDelta()
	rl.CameraYaw(camera, -mouseDelta.X*Sensitivity, 0)
	rl.CameraPitch(camera, -mouseDelta.Y*Sensitivity, 0, 0, 0)

	var movementSpeed float32
	if rl.IsKeyDown(rl.KeyLeftShift) {
		movementSpeed = MovementSpeed * 2
	} else if rl.IsKeyDown(rl.KeyC) {
		movementSpeed = MovementSpeed / 2
	} else {
		movementSpeed = MovementSpeed
	}

	if rl.IsKeyDown(rl.KeyW) {
		rl.CameraMoveForward(camera, movementSpeed, 1)
	}
	if rl.IsKeyDown(rl.KeyA) {
		rl.CameraMoveRight(camera, -movementSpeed, 1)
	}
	if rl.IsKeyDown(rl.KeyS) {
		rl.CameraMoveForward(camera, -movementSpeed, 1)
	}
	if rl.IsKeyDown(rl.KeyD) {
		rl.CameraMoveRight(camera, movementSpeed, 1)
	}

	if rl.IsKeyDown(rl.KeySpace) {
		rl.CameraMoveUp(camera, movementSpeed)
	}
	if rl.IsKeyDown(rl.KeyLeftControl) {
		rl.CameraMoveUp(camera, -movementSpeed)
	}
}

func main() {
	rl.InitWindow(int32(CUBE_SIZE*SCALE), int32(CUBE_SIZE*SCALE), "Fluid Simulation")
	rl.SetTargetFPS(60)

	camera := rl.NewCamera3D(
		rl.NewVector3(0, 0, 0),
		rl.NewVector3(-4, -4, -4),
		rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		120.0,
		rl.CameraPerspective,
	)

	dp := rl.Vector3Zero()

	fluid := NewFluidCube(64, 0, 0, 0.1)

	for !rl.WindowShouldClose() {
		rl.UpdateCamera(&camera, rl.CameraFree)

		rl.BeginDrawing()
		rl.BeginMode3D(camera)

		if rl.IsKeyPressed(rl.KeySpace) {
			fluid.AddDensity(camera.Position, 100)
			fluid.AddVelocity(camera.Position, dp)
		}

		fluid.Step()
		fluid.RenderD(&camera)
		oldPos := camera.Position
		handleMovement(&camera)
		dp = rl.Vector3Subtract(camera.Position, oldPos)

		rl.DrawSphere(rl.NewVector3(0.0, 0.0, 0.0), 20.0, rl.RayWhite)
		rl.DrawPlane(rl.NewVector3(0.0, 0.0, 0.0), rl.NewVector2(100.0, 100.0), rl.Red)

		rl.EndMode3D()
		rl.EndDrawing()
	}

	rl.CloseWindow()
}
