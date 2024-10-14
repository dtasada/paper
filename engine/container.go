package engine

import (
	"fmt"
	"slices"

	m "github.com/dtasada/paper/engine/math"
	"github.com/gen2brain/raylib-go/raylib"
	"github.com/kr/pretty"
)

type Grid struct {
	Columns int
	Rows    int
	Planes  int

	Content map[int]Plane
}

type Bounds struct {
	XMin float32
	XMax float32
	YMin float32
	YMax float32
	ZMin float32
	ZMax float32

	XMinModel rl.Model
	XMaxModel rl.Model
	YMinModel rl.Model
	YMaxModel rl.Model
	ZMinModel rl.Model
	ZMaxModel rl.Model
}

type Container struct {
	Pos      m.V3
	Width    float32
	Height   float32
	Length   float32
	CellSize float32 // Amount of raylib coordinate units per cell

	Bounds Bounds
	Grid   Grid
}

func NewContainer(pos, size m.V3, cellSize float32, shader rl.Shader) Container {
	width := size.X
	height := size.Y
	length := size.Z

	columns := int(width / cellSize)
	rows := int(height / cellSize)
	planes := int(length / cellSize)

	grid := Grid{
		columns,
		rows,
		planes,
		map[int]Plane{},
	}

	count := 0
	for z := -grid.Planes / 2; z <= grid.Planes/2; z++ {
		zi := z * int(cellSize)
		grid.Content[zi] = Plane{}
		count += 1
		for y := -grid.Rows / 2; y <= grid.Rows/2; y++ {
			yi := y * int(cellSize)
			grid.Content[zi][yi] = Row{}
			for x := -grid.Columns / 2; x <= grid.Columns/2; x++ {
				xi := x * int(cellSize)
				grid.Content[zi][yi][xi] = Cell{}
			}
		}
	}

	bounds := Bounds{
		YMin: pos.Y - height/2,
		YMax: pos.Y + height/2,
		XMin: pos.X - width/2,
		XMax: pos.X + width/2,
		ZMin: pos.Z - length/2,
		ZMax: pos.Z + length/2,

		XMinModel: rl.LoadModelFromMesh(rl.GenMeshCube(0.1, height, length)),
		XMaxModel: rl.LoadModelFromMesh(rl.GenMeshCube(0.1, height, length)),
		YMinModel: rl.LoadModelFromMesh(rl.GenMeshCube(width, 0.1, length)),
		YMaxModel: rl.LoadModelFromMesh(rl.GenMeshCube(width, 0.1, length)),
		ZMinModel: rl.LoadModelFromMesh(rl.GenMeshCube(width, height, 0.1)),
		ZMaxModel: rl.LoadModelFromMesh(rl.GenMeshCube(width, height, 0.1)),
	}

	bounds.XMinModel.Materials.Shader = shader
	bounds.XMaxModel.Materials.Shader = shader
	bounds.YMinModel.Materials.Shader = shader
	bounds.YMaxModel.Materials.Shader = shader
	bounds.ZMinModel.Materials.Shader = shader
	bounds.ZMaxModel.Materials.Shader = shader

	return Container{
		Pos:      pos,
		Width:    width,
		Height:   height,
		Length:   length,
		CellSize: cellSize,

		Bounds: bounds,

		Grid: grid,
	}
}

/* Helper function that runs f over every particle adjacent to pa */
func (self *Container) ForAdjacentParticles(pa *Particle, f func(*Particle, *Particle)) {
	cell := self.GetParticleCell(pa)
	targetCell := m.V3Int{}
	d := int(self.CellSize)

	// if statements are to prevent checking non-existing would-be "edge-adjacent" cells
	for dx := -d; dx <= d; dx += d {
		if targetCell.X = cell.X + dx; targetCell.X >= int(self.Bounds.XMin) &&
			targetCell.X <= int(self.Bounds.XMax) {
			for dy := -d; dy <= d; dy += d {
				if targetCell.Y = cell.Y + dy; targetCell.Y >= int(self.Bounds.YMin) &&
					targetCell.Y <= int(self.Bounds.YMax) {
					for dz := -d; dz <= d; dz += d {
						if targetCell.Z = cell.Z + dz; targetCell.Z >= int(self.Bounds.ZMin) &&
							targetCell.Z <= int(self.Bounds.ZMax) {
							if ShowCollisionGrid {
								self.DrawCell(targetCell, rl.Green)
							}

							/* Iterate over cell */
							for _, pb := range self.Grid.Content[targetCell.Z][targetCell.Y][targetCell.X] {
								if pa != pb { // dont interact with itself
									f(pa, pb)
								}
							}
						}
					}
				}
			}
		}
	}
}

/* Helper function that calculates the lambda coefficient in SolveCollision */
func calcLambda(pa, pb *Particle, normal m.V3) float32 {
	var numerator, denominator float32
	qa := m.MatrixMult(
		m.MatrixInvInertia(pa.Inertia),
		m.MatrixGlInverse(m.V3ToMatrix(pa.Pos)),
		m.MatrixBitAnd(m.V3ToMatrix(pa.Lever), m.V3ToMatrix(normal)),
	)
	qb := m.MatrixMult(
		m.MatrixInvInertia(pb.Inertia),
		m.MatrixGlInverse(m.V3ToMatrix(pb.Pos)),
		m.MatrixBitAnd(m.V3ToMatrix(pb.Lever), m.V3ToMatrix(normal)),
	)
	numerator -= m.MatrixBitOr(m.V3ToMatrix(pa.Vel), m.V3ToMatrix(normal))
	numerator += m.MatrixBitOr(m.V3ToMatrix(pb.Vel), m.V3ToMatrix(normal))
	numerator -= m.MatrixBitOr(m.MatrixMult(pa.Inertia, qa), m.V3ToMatrix(pa.AngVel))
	numerator += m.MatrixBitOr(m.MatrixMult(pb.Inertia, qb), m.V3ToMatrix(pb.AngVel))
	denominator += 1 / pa.Mass
	denominator += 1 / pb.Mass
	denominator += m.MatrixBitOr(m.MatrixMult(pa.Inertia, qa), qa)
	denominator += m.MatrixBitOr(m.MatrixMult(pb.Inertia, qb), qb)
	return 2 * numerator / denominator
}

/* Solves collision given cells */
func (self *Container) SolveCollision(pa, pb *Particle) {
	if ShowCollisionLines {
		rl.DrawLine3D(pa.Pos, pb.Pos, rl.Red)
	}

	if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
		normal := rl.Vector3Normalize(m.V3Sub(pa.Pos, pb.Pos))

		λ := calcLambda(pa, pb, normal)
		pa.Vel = m.V3Add(pa.Vel, m.V3MultVal(normal, (λ/pa.Mass)))
		pb.Vel = m.V3Sub(pb.Vel, m.V3MultVal(normal, (λ/pb.Mass)))
		pa.AngVel = m.V3AddMatrix(
			pa.AngVel,
			m.MatrixMultVal(
				m.MatrixMult(m.MatrixInvInertia(pa.Inertia), m.MatrixGlInverse(m.V3ToMatrix(pa.Pos))),
				m.V3BitOr(pa.Lever, normal)*λ,
			),
		)
		pb.AngVel = m.V3SubMatrix(
			pb.AngVel,
			m.MatrixMultVal(
				m.MatrixMult(m.MatrixInvInertia(pb.Inertia), m.MatrixGlInverse(m.V3ToMatrix(pb.Pos))),
				m.V3BitOr(pb.Lever, normal)*λ,
			),
		)
	}

	if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
		fmt.Println(rl.GetTime(), "Collision not resolved")
	}
}

/* Deletes a given particle from its corresponding cell */
func (self *Container) DelParticleFromCell(particle *Particle, cell m.V3Int) {
	for i, p := range self.Grid.Content[cell.Z][cell.Y][cell.X] {
		if particle == p {
			self.Grid.Content[cell.Z][cell.Y][cell.X] = slices.Delete(self.Grid.Content[cell.Z][cell.Y][cell.X], i, i+1)
		}
	}
}

/* Returns a particle's corresponding cell */
func (self *Container) GetParticleCell(particle *Particle) m.V3Int {
	return m.V3Int{
		RoundToCellSize(particle.Pos.X, self.CellSize),
		RoundToCellSize(particle.Pos.Y, self.CellSize),
		RoundToCellSize(particle.Pos.Z, self.CellSize),
	}
}

func (self *Container) DrawCell(cell m.V3Int, color rl.Color) {
	rl.DrawCubeWires(
		m.V3IntToV3(cell),
		self.CellSize,
		self.CellSize,
		self.CellSize,
		color,
	)
}

func (self *Container) DrawBounds() {
	// rl.DrawCubeWires(container.Pos, container.Width, container.Height, container.Length, rl.Red)
	color := rl.Blue
	if !ContainParticles {
		color = rl.Red
	}
	if ShowContainerWalls {
		rl.DrawModel(self.Bounds.XMinModel, rl.NewVector3(self.Bounds.XMin, 0, 0), 1, color)
		rl.DrawModel(self.Bounds.XMaxModel, rl.NewVector3(self.Bounds.XMax, 0, 0), 1, color)
		rl.DrawModel(self.Bounds.YMinModel, rl.NewVector3(0, self.Bounds.YMin, 0), 1, color)
		rl.DrawModel(self.Bounds.YMaxModel, rl.NewVector3(0, self.Bounds.YMax, 0), 1, color)
		rl.DrawModel(self.Bounds.ZMinModel, rl.NewVector3(0, 0, self.Bounds.ZMin), 1, color)
		rl.DrawModel(self.Bounds.ZMaxModel, rl.NewVector3(0, 0, self.Bounds.ZMax), 1, color)
	} else {
		rl.DrawCubeWires(self.Pos, self.Width, self.Height, self.Length, color)
	}
}

func (self *Container) Print() {
	out := map[int]Plane{}

	for z, plane := range self.Grid.Content {
		for y, row := range plane {
			for x, cell := range row {
				if len(cell) != 0 {
					out[z] = Plane{}
					out[z][y] = Row{}
					out[z][y][x] = cell
				}
			}
		}
	}

	for z, plane := range out {
		pretty.Printf("%d: Plane {\n", z)
		for y, row := range plane {
			pretty.Printf("\t%d: Row {\n", y)
			for x, cell := range row {
				pretty.Printf("\t\t%d: Cell {\n", x)
				for _, p := range cell {
					pretty.Printf("\t\t\tParticle {\n")
					pretty.Printf("\t\t\t\tPosition: %v, %v, %v\n", p.Pos.X, p.Pos.Y, p.Pos.Z)
					pretty.Printf("\t\t\t\tVelocity: %v, %v, %v\n", p.Vel.X, p.Vel.Y, p.Vel.Z)
					pretty.Printf("\t\t\t\tRadius: %v\n", p.Radius)
					pretty.Printf("\t\t\t}\n")
				}
				pretty.Printf("\n\t\t}\n")
			}
			pretty.Printf("\n\t}\n")
		}
		pretty.Printf("}\n")
	}
}
