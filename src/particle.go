package src

import (
	// "fmt"
	"slices"

	"github.com/gen2brain/raylib-go/raylib"
	// "github.com/kr/pretty"
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

	/* Bounds checking */
	if bottomBorder := container.Bounds.YMin + self.Radius; self.Pos.Y <= bottomBorder {
		self.Pos.Y = bottomBorder
		self.Vel.Y *= -1 * self.CollisionDamping
	} else if topBorder := container.Bounds.YMax - self.Radius; self.Pos.Y >= topBorder {
		self.Pos.Y = topBorder
		self.Vel.Y *= -1 * self.CollisionDamping
	}

	if leftBorder := container.Bounds.XMin + self.Radius; self.Pos.X <= leftBorder {
		self.Pos.X = leftBorder
		self.Vel.X *= -1 * self.CollisionDamping
	} else if rightBorder := container.Bounds.XMax - self.Radius; self.Pos.X >= rightBorder {
		self.Pos.X = rightBorder
		self.Vel.X *= -1 * self.CollisionDamping
	}

	if shallowBorder := container.Bounds.ZMin + self.Radius; self.Pos.Z <= shallowBorder {
		self.Pos.Z = shallowBorder
		self.Vel.Z *= -1 * self.CollisionDamping
	} else if deepBorder := container.Bounds.ZMax - self.Radius; self.Pos.Z >= deepBorder {
		self.Pos.Z = deepBorder
		self.Vel.Z *= -1 * self.CollisionDamping
	}

	// Keep particle in the right cell
	// If particle not in cell's range, move particle to the right cell
	cell := container.GetParticleCell(self)
	if !slices.Contains(container.Grid.Content[cell.Z][cell.Y][cell.X], self) {
		container.DelParticleFromCell(self)
		container.Grid.Content[cell.Z][cell.Y][cell.X] = append(container.Grid.Content[cell.Z][cell.Y][cell.X], self)
	}

	container.FindCollisions(self)

	rl.DrawModel(self.Model, self.Pos, 1, self.Color)
	// rl.DrawModelWires(self.Model, self.Pos, 1, InvertColor(self.Color))
}
