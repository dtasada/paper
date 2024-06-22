package main

import (
	// "fmt"

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
}

func NewParticle(pos, vel Vector3, radius float32, color rl.Color) Particle {
	return Particle{
		Pos:              pos,
		Vel:              vel,
		Radius:           radius,
		CollisionDamping: 0.8,
		Color:            color,
	}
}

func (self *Particle) Update(bounds *Bounds) {
	dt := rl.GetFrameTime()
	self.Vel.Y -= GRAVITY * dt
	self.Pos = rl.Vector3Add(self.Pos, self.Vel)

	if self.Pos.Y <= bounds.Bottom {
		self.Pos.Y = bounds.Pos.Y + (bounds.Height/2)*GetSign(self.Pos.Y)
		self.Vel.Y *= -1 * self.CollisionDamping
	} else if self.Pos.Y >= bounds.Top {
		self.Pos.Y = bounds.Pos.Y - (bounds.Height/2)*GetSign(self.Pos.Y)
		self.Vel.Y *= -1 * self.CollisionDamping
	}

	if self.Pos.X <= bounds.XLeast {
		self.Pos.X = bounds.Pos.X + (bounds.Width/2)*GetSign(self.Pos.X)
		self.Vel.X *= -1 * self.CollisionDamping
	} else if self.Pos.X >= bounds.XMost {
		self.Pos.X = bounds.Pos.X - (bounds.Width/2)*GetSign(self.Pos.X)
		self.Vel.X *= -1 * self.CollisionDamping
	}

	if self.Pos.Z <= bounds.ZLeast {
		self.Pos.Z = bounds.Pos.Z + (bounds.Length/2)*GetSign(self.Pos.Z)
		self.Vel.Z *= -1 * self.CollisionDamping
	} else if self.Pos.Z >= bounds.ZMost {
		self.Pos.Z = bounds.Pos.Z - (bounds.Length/2)*GetSign(self.Pos.Z)
		self.Vel.Z *= -1 * self.CollisionDamping
	}

	rl.DrawSphere(self.Pos, self.Radius, self.Color)
}

type Bounds struct {
	Pos    Vector3
	Width  float32
	Height float32
	Length float32

	Bottom float32
	Top    float32
	XLeast float32
	XMost  float32
	ZLeast float32
	ZMost  float32
}

func NewBounds(pos Vector3, width, height, length float32) Bounds {
	return Bounds{
		Pos:    pos,
		Width:  width,
		Height: height,
		Length: length,

		Bottom: pos.Y - height/2,
		Top:    pos.Y + height/2,
		XLeast: pos.X - width/2,
		XMost:  pos.X + width/2,
		ZLeast: pos.Z - length/2,
		ZMost:  pos.Z + length/2,
	}
}
