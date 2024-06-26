package src

import (
	"slices"

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
	YMin float32
	YMax float32
	XMin float32
	XMax float32
	ZMin float32
	ZMax float32
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

func NewContainer(pos, size Vector3, cellsToSize float32) Container {
	width := size.X
	height := size.Y
	length := size.Z

	columns := int(width / cellsToSize)
	rows := int(height / cellsToSize)
	planes := int(length / cellsToSize)

	return Container{
		Pos:      pos,
		Width:    width,
		Height:   height,
		Length:   length,
		CellSize: cellsToSize,

		Bounds: Bounds{
			YMin: pos.Y - height/2,
			YMax: pos.Y + height/2,
			XMin: pos.X - width/2,
			XMax: pos.X + width/2,
			ZMin: pos.Z - length/2,
			ZMax: pos.Z + length/2,
		},

		Grid: Grid{
			columns,
			rows,
			planes,
			map[int]Plane{},
		},
	}
}

/* Find collision between a particle and any particle in its contiguous cells */
func (self *Container) FindCollisions(particle *Particle) {
	cell := self.GetParticleCell(particle)
	// self.DrawCell(cell, rl.Green)

	// if statements are to prevent checking non-existing would-be "edge-adjacent" cells
	targetCell := Vector3Int{}
	d := int(self.CellSize)
	for dx := -d; dx <= d; dx += d {
		if targetCell.X = cell.X + dx; targetCell.X >= int(self.Bounds.XMin) && targetCell.X <= int(self.Bounds.XMax) {
			for dy := -d; dy <= d; dy += d {
				if targetCell.Y = cell.Y + dy; targetCell.Y >= int(self.Bounds.YMin) && targetCell.Y <= int(self.Bounds.YMax) {
					for dz := -d; dz <= d; dz += d {
						if targetCell.Z = cell.Z + dz; targetCell.Z >= int(self.Bounds.ZMin) && targetCell.Z <= int(self.Bounds.ZMax) {
							// self.DrawCell(targetCell, rl.Green)
							self.SolveCollision(particle, targetCell)
						}
					}
				}
			}
		}
	}
}

/* Solves collision given cells */
func (self *Container) SolveCollision(pa *Particle, cellPos Vector3Int) {
	cell := self.Grid.Content[cellPos.Z][cellPos.Y][cellPos.X]
	pretty.Print()

	for _, pb := range cell {
		if pa != pb {
			if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
				axis := rl.Vector3Subtract(pa.Pos, pb.Pos)
				norm := rl.Vector3Normalize(axis)
				compensationDistance := pa.Radius + pb.Radius - rl.Vector3Length(axis)
				norm = Vector3MultiplyValue(norm, compensationDistance/2)
				pa.Pos = rl.Vector3Add(pa.Pos, norm)
				pb.Pos = rl.Vector3Subtract(pb.Pos, norm)
			}
		}
	}
}

func (self *Container) DelParticleFromCell(particle *Particle) {
	cell := self.GetParticleCell(particle)
	for i, p := range self.Grid.Content[cell.Z][cell.Y][cell.X] {
		if particle == p {
			self.Grid.Content[cell.Z][cell.Y][cell.X] = slices.Delete(self.Grid.Content[cell.Z][cell.Y][cell.X], i, i+1)
		}
	}
}

/* Returns a particle's corresponding cell */
func (self *Container) GetParticleCell(particle *Particle) Vector3Int {
	return Vector3Int{
		int(f32(FloorTens(particle.Pos.X)) - self.CellSize/2.0),
		int(f32(FloorTens(particle.Pos.Y)) - self.CellSize/2.0),
		int(f32(FloorTens(particle.Pos.Z)) - self.CellSize/2.0),
	}
}

func (self *Container) DrawCell(cell Vector3Int, color rl.Color) {
	rl.DrawCubeWires(
		rl.NewVector3(
			f32(cell.X),
			f32(cell.Y),
			f32(cell.Z),
		),
		self.CellSize,
		self.CellSize,
		self.CellSize,
		color,
	)
}
