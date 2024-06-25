package src

import (
	"github.com/gen2brain/raylib-go/raylib"
)

type Particle struct {
	Pos              rl.Vector3
	Vel              rl.Vector3
	Radius           float32
	CollisionDamping float32
	Color            rl.Color
	Model            rl.Model
}

func NewParticle(pos, vel Vector3, radius float32, color rl.Color, shader rl.Shader) Particle {
	model := rl.LoadModelFromMesh(rl.GenMeshSphere(radius, 32, 32))
	model.Materials.Shader = shader
	return Particle{
		Pos:              pos,
		Vel:              vel,
		Radius:           radius,
		CollisionDamping: 0.8,
		Color:            color,
		Model:            model,
	}
}

func (self *Particle) Update(container *Container, particles *[]*Particle) {
	dt := rl.GetFrameTime()
	self.Vel.Y -= GRAVITY * dt
	self.Vel.X -= GRAVITY * dt
	self.Vel.Z -= GRAVITY * dt
	self.Pos = rl.Vector3Add(self.Pos, self.Vel)

	/* Bound checking */
	if self.Pos.Y <= container.Bounds.YMin+self.Radius {
		self.Pos.Y = container.Bounds.YMin + self.Radius
		self.Vel.Y *= -1 * self.CollisionDamping
	} else if self.Pos.Y >= container.Bounds.YMax-self.Radius {
		self.Pos.Y = container.Bounds.YMax - self.Radius
		self.Vel.Y *= -1 * self.CollisionDamping
	}

	if self.Pos.X <= container.Bounds.XMin+self.Radius {
		self.Pos.X = container.Bounds.XMin + self.Radius
		self.Vel.X *= -1 * self.CollisionDamping
	} else if self.Pos.X >= container.Bounds.XMax-self.Radius {
		self.Pos.X = container.Bounds.XMax - self.Radius
		self.Vel.X *= -1 * self.CollisionDamping
	}

	if self.Pos.Z <= container.Bounds.ZMin+self.Radius {
		self.Pos.Z = container.Bounds.ZMin + self.Radius
		self.Vel.Z *= -1 * self.CollisionDamping
	} else if self.Pos.Z >= container.Bounds.ZMax-self.Radius {
		self.Pos.Z = container.Bounds.ZMax - self.Radius
		self.Vel.Z *= -1 * self.CollisionDamping
	}

	container.FindCollisions(self)

	rl.DrawModel(self.Model, self.Pos, 1, self.Color)
	// rl.DrawModelWires(self.Model, self.Pos, 1, InvertColor(self.Color))
}
