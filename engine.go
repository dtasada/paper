package main

import (
	"fmt"

	"github.com/gen2brain/raylib-go/raylib"
)

type Vector2 = rl.Vector2
type Vector3 = rl.Vector3

const GRAVITY float32 = 9.81

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
}

func NewParticle(pos, vel Vector3, radius float32, color rl.Color, id int) *Particle {
	return &Particle{
		Pos:              pos,
		Vel:              vel,
		Radius:           radius,
		CollisionDamping: 0.8,
		Color:            color,
		Id:               id,
	}
}

func (self *Particle) Update(bounds *Bounds) {
	self.Vel.Y -= GRAVITY * rl.GetFrameTime()
	self.Pos = rl.Vector3Add(self.Pos, self.Vel)

	if self.Pos.Y < bounds.YMin {
		self.Pos.Y = bounds.Pos.Y + (bounds.Height/2)*GetSign(self.Pos.Y)
		self.Vel.Y *= -1 * self.CollisionDamping
	} else if self.Pos.Y > bounds.YMax {
		self.Pos.Y = bounds.Pos.Y - (bounds.Height/2)*GetSign(self.Pos.Y)
		self.Vel.Y *= -1 * self.CollisionDamping
	}

	if self.Pos.X < bounds.XMin {
		self.Pos.X = bounds.Pos.X + (bounds.Width/2)*GetSign(self.Pos.X)
		self.Vel.X *= -1 * self.CollisionDamping
	} else if self.Pos.X > bounds.XMax {
		self.Pos.X = bounds.Pos.X - (bounds.Width/2)*GetSign(self.Pos.X)
		self.Vel.X *= -1 * self.CollisionDamping
	}

	if self.Pos.Z < bounds.ZMin {
		self.Pos.Z = bounds.Pos.Z + (bounds.Length/2)*GetSign(self.Pos.Z)
		self.Vel.Z *= -1 * self.CollisionDamping
	} else if self.Pos.Z > bounds.ZMax {
		self.Pos.Z = bounds.Pos.Z - (bounds.Length/2)*GetSign(self.Pos.Z)
		self.Vel.Z *= -1 * self.CollisionDamping
	}

	rl.DrawSphere(self.Pos, self.Radius, self.Color)
}

func (self *Particle) Collision() {}

type Bounds struct {
	Pos    Vector3
	Width  float32
	Height float32
	Length float32

	YMin float32
	YMax float32
	XMin float32
	XMax float32
	ZMin float32
	ZMax float32
}

func NewBounds(pos Vector3, width, height, length float32) Bounds {
	fmt.Print()
	return Bounds{
		Pos:    pos,
		Width:  width,
		Height: height,
		Length: length,

		YMin: pos.Y - height/2,
		YMax: pos.Y + height/2,
		XMin: pos.X - width/2,
		XMax: pos.X + width/2,
		ZMin: pos.Z - length/2,
		ZMax: pos.Z + length/2,
	}
}

type Grid struct {
	Width  float32
	Height float32
	Length float32
}
