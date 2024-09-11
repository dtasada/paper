package src

import (
	"fmt"
	"slices"

	m "github.com/dtasada/paper/src/math"
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

/* f is a function to run over every adjacent cell to a given particle */
func (self *Container) ForAdjacentParticles(pa *Particle, f func(*Particle, *Particle)) {
	cell := self.GetParticleCell(pa)
	targetCell := m.V3Int{}
	d := int(self.CellSize)

	// if statements are to prevent checking non-existing would-be "edge-adjacent" cells
	for dx := -d; dx <= d; dx += d {
		if targetCell.X = cell.X + dx; targetCell.X >= int(self.Bounds.XMin) && targetCell.X <= int(self.Bounds.XMax) {
			for dy := -d; dy <= d; dy += d {
				if targetCell.Y = cell.Y + dy; targetCell.Y >= int(self.Bounds.YMin) && targetCell.Y <= int(self.Bounds.YMax) {
					for dz := -d; dz <= d; dz += d {
						if targetCell.Z = cell.Z + dz; targetCell.Z >= int(self.Bounds.ZMin) && targetCell.Z <= int(self.Bounds.ZMax) {
							if ShowCollisionGrid {
								self.DrawCell(targetCell, rl.Green)
							}

							/* Iterate over cell */
							for _, pb := range self.Grid.Content[targetCell.Z][targetCell.Y][targetCell.X] {
								if pa != pb { // dont collide with itself
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

func calcLambda(pa, pb *Particle) (m.V3, m.V3) {
	// Getting requirements
	d := rl.Vector3Distance(pa.Pos, pb.Pos)
	between := rl.Vector3Subtract(pb.Pos, pa.Pos)
	collisionNormal := rl.Vector3Normalize(between)
	pointOfContactA := m.V3MultVal(between, pa.Radius/d)  // test point of contact
	pointOfContactB := m.V3MultVal(between, -pb.Radius/d) // the minus sign is to flip the whole vector so it's pointing the other way

	pa.Q = m.V3MultMatrix(
		rl.Vector3CrossProduct(pointOfContactA, collisionNormal),
		m.MatrixMult(m.MatrixInvert(pa.Inertia), m.MatrixInvert(pa.Orientation)),
	)
	pb.Q = m.V3MultMatrix(
		rl.Vector3CrossProduct(pointOfContactB, collisionNormal),
		m.MatrixMult(m.MatrixInvert(pb.Inertia), m.MatrixInvert(pb.Orientation)),
	)

	// Actual equations
	nv1_nv2 := m.V3Mult(collisionNormal, rl.Vector3Subtract(pa.Vel, pb.Vel))
	wiqA := m.V3MultMatrix(m.V3Mult(pa.AngVel, pa.Q), pa.Inertia)
	wiqB := m.V3MultMatrix(m.V3Mult(pb.AngVel, pb.Q), pb.Inertia)
	qiqA := m.V3Mult(pa.Q, pa.AngVel) // Check the Q*I*Q
	qiB := m.V3Mult(pb.Q, pb.AngVel)
	denumerator := m.V3Add(nv1_nv2, wiqA, m.V3MultVal(wiqB, -1))
	denominator := m.V3Add(m.V3MultVal(m.V3Mult(collisionNormal, collisionNormal), (1/pa.Mass+1/pb.Mass)), qiqA, qiB)
	λ := m.V3MultVal(rl.Vector3Divide(denumerator, denominator), 2)

	return λ, collisionNormal
}

/* Solves collision given cells */
func (self *Container) SolveCollision(pa, pb *Particle) {
	fmt.Println("solvecollision")
	if ShowCollisionLines {
		rl.DrawLine3D(pa.Pos, pb.Pos, rl.Red)
	}

	if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
		paQBefore := pa.Q
		pbQBefore := pb.Q
		λ, n := calcLambda(pa, pb)
		pa.Vel = m.V3Sub(pa.Vel, m.V3Mult(m.V3DivVal(λ, pa.Mass), n))
		pb.Vel = m.V3Add(pb.Vel, m.V3Mult(m.V3DivVal(λ, pb.Mass), n))
		pa.AngVel = m.V3Sub(pa.AngVel, m.V3Sub(paQBefore, pa.Q))
		pb.AngVel = m.V3Add(pb.AngVel, m.V3Sub(pbQBefore, pb.Q))
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
	rl.DrawModel(self.Bounds.XMinModel, rl.NewVector3(self.Bounds.XMin, 0, 0), 1, color)
	rl.DrawModel(self.Bounds.XMaxModel, rl.NewVector3(self.Bounds.XMax, 0, 0), 1, color)
	rl.DrawModel(self.Bounds.YMinModel, rl.NewVector3(0, self.Bounds.YMin, 0), 1, color)
	rl.DrawModel(self.Bounds.YMaxModel, rl.NewVector3(0, self.Bounds.YMax, 0), 1, color)
	rl.DrawModel(self.Bounds.ZMinModel, rl.NewVector3(0, 0, self.Bounds.ZMin), 1, color)
	rl.DrawModel(self.Bounds.ZMaxModel, rl.NewVector3(0, 0, self.Bounds.ZMax), 1, color)
}
