package main

import (
	// "fmt"

	"github.com/gen2brain/raylib-go/raylib"
)

type Vector2 = rl.Vector2
type Vector3 = rl.Vector3
type GridCell = []*Particle

const GRAVITY float32 = 1

func InvertColor(c rl.Color) rl.Color {
	return rl.NewColor(
		255-c.R,
		255-c.G,
		255-c.B,
		255-c.A,
	)
}

func NewVector3FromInt(x, y, z int) Vector3 {
	return rl.NewVector3(
		float32(x),
		float32(y),
		float32(z),
	)
}

func Vector3ToInt(vec Vector3) struct{ X, Y, Z int } {
	return struct {
		X, Y, Z int
	}{
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

type Particle struct {
	Pos              rl.Vector3
	Vel              rl.Vector3
	Radius           float32
	CollisionDamping float32
	Color            rl.Color
	Id               int
	Model            rl.Model
}

func NewParticle(pos, vel Vector3, radius float32, color rl.Color, id int, shader rl.Shader) Particle {
	model := rl.LoadModelFromMesh(rl.GenMeshSphere(radius, 32, 32))
	model.Materials.Shader = shader
	return Particle{
		Pos:              pos,
		Vel:              vel,
		Radius:           radius,
		CollisionDamping: 0.8,
		Color:            color,
		Id:               id,
		Model:            model,
	}
}

func (self *Particle) Update(container *Container, particles *[]*Particle) {
	dt := rl.GetFrameTime()
	self.Vel.Y -= GRAVITY * dt
	self.Pos = rl.Vector3Add(self.Pos, self.Vel)

	/* Bound checking */
	if self.Pos.Y <= container.Bounds.YMin {
		self.Pos.Y = container.Pos.Y + (container.Height/2)*GetSign(self.Pos.Y)
		self.Vel.Y *= -1 * self.CollisionDamping
	} else if self.Pos.Y >= container.Bounds.YMax {
		self.Pos.Y = container.Pos.Y - (container.Height/2)*GetSign(self.Pos.Y)
		self.Vel.Y *= -1 * self.CollisionDamping
	}

	if self.Pos.X <= container.Bounds.XMin {
		self.Pos.X = container.Pos.X + (container.Width/2)*GetSign(self.Pos.X)
		self.Vel.X *= -1 * self.CollisionDamping
	} else if self.Pos.X >= container.Bounds.XMax {
		self.Pos.X = container.Pos.X - (container.Width/2)*GetSign(self.Pos.X)
		self.Vel.X *= -1 * self.CollisionDamping
	}

	if self.Pos.Z <= container.Bounds.ZMin {
		self.Pos.Z = container.Pos.Z + (container.Length/2)*GetSign(self.Pos.Z)
		self.Vel.Z *= -1 * self.CollisionDamping
	} else if self.Pos.Z >= container.Bounds.ZMax {
		self.Pos.Z = container.Pos.Z - (container.Length/2)*GetSign(self.Pos.Z)
		self.Vel.Z *= -1 * self.CollisionDamping
	}

	rl.DrawSphere(self.Pos, self.Radius, self.Color)
	rl.DrawSphereWires(self.Pos, self.Radius, 32, 32, InvertColor(self.Color))
}

type Grid struct {
	XCount int
	YCount int
	ZCount int

	Content [][][]GridCell
}

func (self *Grid) FindCollisions() {
	for x := 1; x <= self.XCount; x++ {
		for y := 1; y <= self.YCount; y++ {
			for z := 1; z <= self.ZCount; z++ {
				for dx := -1; dx <= 1; dx++ {
					for dy := -1; dy <= 1; dy++ {
						for dz := -1; dz <= 1; dz++ {
							self.SolveCollision(
								NewVector3FromInt(x, y, z),
								NewVector3FromInt(x+dx, y+dy, z+dz),
							)
						}
					}
				}
			}
		}
	}
}

func (self *Grid) SolveCollision(cellACoords, cellBCoords rl.Vector3) {
	cai := Vector3ToInt(cellACoords) // to spare the int() everywhere
	cbi := Vector3ToInt(cellBCoords)
	cellA := self.Content[cai.Z][cai.Y][cai.Z]
	cellB := self.Content[cbi.Z][cbi.Y][cbi.Z]

	for _, pa := range cellA {
		for _, pb := range cellB {
			if pa != nil && pb != nil && pa != pb {
				if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
					compensationDistance := pa.Radius + pb.Radius - rl.Vector3Distance(pa.Pos, pb.Pos)
					pa.Pos = rl.Vector3SubtractValue(pa.Pos, compensationDistance/2.0)
					pb.Pos = rl.Vector3AddValue(pb.Pos, compensationDistance/2.0)
				}
			}
		}
	}
}

type Bounds struct {
	YMin float32
	YMax float32
	XMin float32
	XMax float32
	ZMin float32
	ZMax float32
}

type Container struct {
	Pos    Vector3
	Width  float32
	Height float32
	Length float32

	Bounds Bounds
	Grid   Grid
}

func NewContainer(pos Vector3, width, height, length float32) Container {
	return Container{
		Pos:    pos,
		Width:  width,
		Height: height,
		Length: length,

		Bounds: Bounds{
			YMin: pos.Y - height/2,
			YMax: pos.Y + height/2,
			XMin: pos.X - width/2,
			XMax: pos.X + width/2,
			ZMin: pos.Z - length/2,
			ZMax: pos.Z + length/2,
		},

		Grid: Grid{10, 10, 10, [][][]GridCell{}},
	}
}
