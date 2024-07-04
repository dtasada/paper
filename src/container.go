package src

import (
	"slices"

	"github.com/gen2brain/raylib-go/raylib"
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
	Pos      Vector3
	Width    float32
	Height   float32
	Length   float32
	CellSize float32 // Amount of raylib coordinate units per cell

	Bounds Bounds
	Grid   Grid
}

func NewContainer(pos, size Vector3, cellSize float32, shader rl.Shader) Container {
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
func (self *Container) ForAdjacentCells(pa *Particle, f func(*Particle, *Particle)) {
	cell := self.GetParticleCell(pa)
	targetCell := Vector3Int{}
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
								if pa != pb {
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

/* Solves collision given cells */
func (self *Container) SolveCollision(pa *Particle, pb *Particle) {
	if ShowCollisionLines {
		rl.DrawLine3D(pa.Pos, pb.Pos, rl.Red)
	}

	if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
		axis := rl.Vector3Subtract(pa.Pos, pb.Pos)
		norm := rl.Vector3Normalize(axis)
		dist := pa.Radius + pb.Radius - rl.Vector3Length(axis)
		norm = Vector3MultiplyValue(norm, dist/2)

		dt := rl.GetFrameTime()

		pa.Pos = rl.Vector3Add(pa.Pos, Vector3MultiplyValue(norm, dt))
		pb.Pos = rl.Vector3Subtract(pb.Pos, Vector3MultiplyValue(norm, dt))

		pa.Vel = Vector3MultiplyValue(pa.Vel, -1*pa.CollisionDamping*dt)
		pb.Vel = Vector3MultiplyValue(pb.Vel, -1*pb.CollisionDamping*dt)
	}
}

/* Deletes a given particle from its corresponding cell */
func (self *Container) DelParticleFromCell(particle *Particle, cell Vector3Int) {
	for i, p := range self.Grid.Content[cell.Z][cell.Y][cell.X] {
		if particle == p {
			self.Grid.Content[cell.Z][cell.Y][cell.X] = slices.Delete(self.Grid.Content[cell.Z][cell.Y][cell.X], i, i+1)
		}
	}
}

/* Returns a particle's corresponding cell */
func (self *Container) GetParticleCell(particle *Particle) Vector3Int {
	return Vector3Int{
		RoundToCellSize(particle.Pos.X, self.CellSize),
		RoundToCellSize(particle.Pos.Y, self.CellSize),
		RoundToCellSize(particle.Pos.Z, self.CellSize),
	}
}

func (self *Container) DrawCell(cell Vector3Int, color rl.Color) {
	rl.DrawCubeWires(
		Vector3IntToVector3(cell),
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
