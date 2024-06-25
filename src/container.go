package src

import (
	"fmt"

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
	cell := Vector3Int{ // find particle's corresponding cell
		int(f32(FloorTens(particle.Pos.X)) - self.CellSize/2.0),
		int(f32(FloorTens(particle.Pos.Y)) - self.CellSize/2.0),
		int(f32(FloorTens(particle.Pos.Z)) - self.CellSize/2.0),
	}
	rl.DrawCubeWires(
		rl.NewVector3(
			f32(cell.X),
			f32(cell.Y),
			f32(cell.Z),
		),
		self.CellSize,
		self.CellSize,
		self.CellSize,
		rl.Green,
	)

	// if statements are to prevent checking non-existing would-be "edge-adjacent" cells
	targetCell := Vector3Int{}
	d := int(self.CellSize)
	for dx := -d; dx <= d; dx += d {
		if targetCell.X = cell.X + dx; targetCell.X >= int(self.Bounds.XMin) && targetCell.X <= int(self.Bounds.XMax) {
			for dy := -d; dy <= d; dy += d {
				if targetCell.Y = cell.Y + dy; targetCell.Y >= int(self.Bounds.YMin) && targetCell.Y <= int(self.Bounds.YMax) {
					for dz := -d; dz <= d; dz += d {
						if targetCell.Z = cell.Z + dz; targetCell.Z >= int(self.Bounds.ZMin) && targetCell.Z <= int(self.Bounds.ZMax) {
							rl.DrawCubeWires(
								rl.NewVector3(
									f32(targetCell.X),
									f32(targetCell.Y),
									f32(targetCell.Z),
								),
								self.CellSize,
								self.CellSize,
								self.CellSize,
								rl.Green,
							)
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
	pretty.Println("cell:", cell)

	for _, pb := range cell {
		if pa != pb {
			if rl.CheckCollisionSpheres(pa.Pos, pa.Radius, pb.Pos, pb.Radius) {
				fmt.Println("collide!", rl.GetTime())
				compensationDistance := pa.Radius + pb.Radius - rl.Vector3Distance(pa.Pos, pb.Pos)
				pa.Pos = rl.Vector3SubtractValue(pa.Pos, compensationDistance*pa.CollisionDamping*pb.CollisionDamping/2.0)
				pb.Pos = rl.Vector3AddValue(pb.Pos, compensationDistance*pb.CollisionDamping*pa.CollisionDamping/2.0)
			}
		}
	}
}
