package src

import (
	"math/rand"

	"github.com/gen2brain/raylib-go/raylib"
)

type Particle struct {
	Pos              Vector3
	Vel              Vector3
	Radius           float32
	CollisionDamping float32
	Color            rl.Color
	Model            rl.Model
}

func NewParticle(pos, vel Vector3, radius, collisionDamping float32, color rl.Color, shader rl.Shader) Particle {
	model := rl.LoadModelFromMesh(rl.GenMeshSphere(radius, 32, 32))
	model.Materials.Shader = shader
	return Particle{
		Pos:              pos,
		Vel:              vel,
		Radius:           radius,
		CollisionDamping: collisionDamping,
		Color:            color,
		Model:            model,
	}
}

func CreateParticle(container *Container, particles *[]*Particle, lightShader rl.Shader) {
	p := NewParticle(
		rl.NewVector3(
			rand.Float32()*container.Width-container.Width/2,
			rand.Float32()*container.Height-container.Height/2,
			rand.Float32()*container.Length-container.Length/2,
		), // random position in grid
		rl.Vector3Zero(),
		container.CellSize/2,
		0.1,
		RandomColor(),
		lightShader,
	)

	(*particles) = append(*particles, &p) // add to particle arraylist
	z := RoundToCellSize(float32(rand.Intn(container.Grid.Planes)-container.Grid.Planes/2), container.CellSize)
	y := RoundToCellSize(float32(rand.Intn(container.Grid.Columns)-container.Grid.Columns/2), container.CellSize)
	x := RoundToCellSize(float32(rand.Intn(container.Grid.Rows)-container.Grid.Rows/2), container.CellSize)
	container.Grid.Content[z][y][x] = append(container.Grid.Content[z][y][x], &p) // add to bounds.Grid index
}

func (self *Particle) Update(container *Container, particles *[]*Particle) {
	oldCell := container.GetParticleCell(self)

	dt := rl.GetFrameTime()
	self.Vel.Y -= Gravity * dt
	self.Vel.X -= Gravity * dt
	self.Vel.Z -= Gravity * dt

	targetPos := rl.Vector3Add(self.Pos, Vector3MultiplyValue(self.Vel, dt))
	self.Pos = targetPos
	// TODO: CHECK FOR OBSTACLES BETWEEN FRAMES

	/* Bounds checking */
	if bottomBorder := container.Bounds.YMin + self.Radius; self.Pos.Y <= bottomBorder {
		self.Pos.Y = bottomBorder
		self.Vel.Y *= -1 * self.CollisionDamping * dt
	} else if topBorder := container.Bounds.YMax - self.Radius; self.Pos.Y >= topBorder {
		self.Pos.Y = topBorder
		self.Vel.Y *= -1 * self.CollisionDamping * dt
	}

	if leftBorder := container.Bounds.XMin + self.Radius; self.Pos.X <= leftBorder {
		self.Pos.X = leftBorder
		self.Vel.X *= -1 * self.CollisionDamping * dt
	} else if rightBorder := container.Bounds.XMax - self.Radius; self.Pos.X >= rightBorder {
		self.Pos.X = rightBorder
		self.Vel.X *= -1 * self.CollisionDamping * dt
	}

	if shallowBorder := container.Bounds.ZMin + self.Radius; self.Pos.Z <= shallowBorder {
		self.Pos.Z = shallowBorder
		self.Vel.Z *= -1 * self.CollisionDamping * dt
	} else if deepBorder := container.Bounds.ZMax - self.Radius; self.Pos.Z >= deepBorder {
		self.Pos.Z = deepBorder
		self.Vel.Z *= -1 * self.CollisionDamping * dt
	}

	self.CorrectCell(container, oldCell)

	/* Find collision between a particle and any particle in its contiguous cells */
	container.ForAdjacentCells(self, container.SolveCollision)

	if ShowParticleCells {
		container.DrawCell(container.GetParticleCell(self), rl.White)
	}

	rl.DrawModel(self.Model, self.Pos, 1, self.Color)
	// rl.DrawModelWires(self.Model, self.Pos, 1, InvertColor(self.Color))
}

/* Move particle to its new correct cell */
func (self *Particle) CorrectCell(container *Container, oldCell Vector3Int) {
	newCell := container.GetParticleCell(self) // get current cell

	if oldCell != newCell { // if cell doesn't contain particle anymore
		container.DelParticleFromCell(self, oldCell) // delete self from old cell

		// container cell might be empty? so making sure cell exists
		if container.Grid.Content[newCell.Z] == nil {
			container.Grid.Content[newCell.Z] = Plane{}
		}
		if container.Grid.Content[newCell.Z][newCell.Y] == nil {
			container.Grid.Content[newCell.Z][newCell.Y] = Row{}
		}

		container.Grid.Content[newCell.Z][newCell.Y][newCell.X] = append(container.Grid.Content[newCell.Z][newCell.Y][newCell.X], self) // add self to bounds.Grid index
	}
}
