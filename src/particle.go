package src

import (
	"math/rand"

	m "github.com/dtasada/paper/src/math"
	"github.com/gen2brain/raylib-go/raylib"
)

type Particle struct {
	Pos              m.V3
	Vel              m.V3
	AngVel           m.V3
	Lever            m.V3
	Torque           m.V3
	Orientation      rl.Matrix
	Inertia          rl.Matrix
	Radius           float32
	Mass             float32
	CollisionDamping float32
	Color            rl.Color
	Model            rl.Model
}

func NewParticle(
	pos, vel m.V3,
	radius, mass, collisionDamping float32,
	color rl.Color,
	shader rl.Shader,
) Particle {
	model := rl.LoadModelFromMesh(rl.GenMeshSphere(radius, 32, 32))
	model.Materials.Shader = shader
	return Particle{
		Pos:              pos,
		Vel:              vel,
		AngVel:           rl.Vector3Zero(),
		Lever:            rl.Vector3Zero(),
		Torque:           rl.Vector3Zero(),
		Orientation:      rl.MatrixIdentity(),
		Inertia:          rl.MatrixIdentity(),
		Radius:           radius,
		Mass:             mass,
		CollisionDamping: collisionDamping,
		Color:            color,
		Model:            model,
	}
}

func CreateParticle(container *Container, particles *[]*Particle, lightShader rl.Shader) {
	p := NewParticle(
		rl.NewVector3(
			container.Width*(rand.Float32()-0.5),
			container.Height*(rand.Float32()-0.5),
			container.Length*(rand.Float32()-0.5),
		), // random position in grid
		// rl.Vector3Zero(),
		rl.NewVector3(0, 0, 0),
		container.CellSize/2,
		m.Pow(container.CellSize/2, 2),
		0.0,
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

	{ /* Update particle properties*/
		self.Vel.Y -= Gravity
		self.Pos = m.V3Add(self.Pos, self.Vel)

		self.AngVel = m.V3AddMatrix(
			self.AngVel,
			m.MatrixMult(
				m.MatrixInvInertia(self.Inertia),
				m.MatrixMult(
					m.MatrixGlInverse(m.V3ToMatrix(self.Pos)),
					m.MatrixSub(m.V3ToMatrix(self.Torque), m.MatrixGlDerive(m.V3ToMatrix(self.AngVel))),
					self.Inertia,
					m.V3ToMatrix(self.AngVel),
				),
			),
		)

	}

	{ /* Bounds checking */
		if bottomBorder := container.Bounds.YMin + self.Radius; self.Pos.Y <= bottomBorder {
			self.Pos.Y = bottomBorder
			self.Vel.Y *= -self.CollisionDamping
		} else if topBorder := container.Bounds.YMax - self.Radius; self.Pos.Y >= topBorder {
			self.Pos.Y = topBorder
			self.Vel.Y *= -self.CollisionDamping
		}

		if leftBorder := container.Bounds.XMin + self.Radius; self.Pos.X <= leftBorder {
			self.Pos.X = leftBorder
			self.Vel.X *= -self.CollisionDamping
		} else if rightBorder := container.Bounds.XMax - self.Radius; self.Pos.X >= rightBorder {
			self.Pos.X = rightBorder
			self.Vel.X *= -self.CollisionDamping
		}

		if shallowBorder := container.Bounds.ZMin + self.Radius; self.Pos.Z <= shallowBorder {
			self.Pos.Z = shallowBorder
			self.Vel.Z *= -self.CollisionDamping
		} else if deepBorder := container.Bounds.ZMax - self.Radius; self.Pos.Z >= deepBorder {
			self.Pos.Z = deepBorder
			self.Vel.Z *= -self.CollisionDamping
		}
	}

	self.CorrectCell(container, oldCell)

	/* Find collision between a particle and any particle in its contiguous cells */
	container.ForAdjacentParticles(self, container.SolveCollision)

	{ /* Render */
		if ShowParticleCells {
			container.DrawCell(container.GetParticleCell(self), rl.White)
		}

		if ShowCellModels {
			rl.DrawModel(self.Model, self.Pos, 1, self.Color)
		} else {
			rl.DrawSphere(self.Pos, self.Radius*0.05, self.Color)
			rl.DrawLine3D(self.Pos, m.V3Add(self.Pos, self.Vel), self.Color)
		}
	}
}

/* Move particle to its new correct cell */
func (self *Particle) CorrectCell(container *Container, oldCell m.V3Int) {
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
