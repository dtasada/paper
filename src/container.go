package src

import (
	"fmt"

	"github.com/gen2brain/raylib-go/raylib"
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
	Pos         Vector3
	Width       float32
	Height      float32
	Length      float32
	CellsToSize float32 // Amount of cells per rl coordinate

	Bounds Bounds
	Grid   Grid
}

func NewContainer(pos, size Vector3, cellsToSize float32) Container {
	width := size.X
	height := size.Y
	length := size.Z

	columns := int(cellsToSize * width)
	rows := int(cellsToSize * height)
	planes := int(cellsToSize * length)

	return Container{
		Pos:         pos,
		Width:       width,
		Height:      height,
		Length:      length,
		CellsToSize: cellsToSize,

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
			map[int]map[int]map[int]GridCell{},
		},
	}
}

/* Find collision between a particle and any particle in its contiguous cells */
func (self *Container) FindCollisions(particle *Particle) {
	cell := Vector3Int{ // find particle's corresponding cell
		floorToZero(particle.Pos.X),
		floorToZero(particle.Pos.Y),
		floorToZero(particle.Pos.Z),
	}

	// if statements are to prevent checking non-existing would-be "edge-adjacent" cells
	targetCell := Vector3Int{}
	for dx := -1; dx <= 1; dx++ {
		if targetCell.X = cell.X + dx; targetCell.X >= 0 && targetCell.X <= self.Grid.Rows {
			for dy := -1; dy <= 1; dy++ {
				if targetCell.Y = cell.Y + dy; targetCell.Y >= 0 && targetCell.Y <= self.Grid.Columns {
					for dz := -1; dz <= 1; dz++ {
						if targetCell.Z = cell.Z + dz; targetCell.Z >= 0 && targetCell.Z <= self.Grid.Planes {
							self.SolveCollision(particle, targetCell)
							rl.DrawCubeWires(
								rl.NewVector3(
									f32(targetCell.X*self.Grid.Rows)-self.Width/2,
									f32(targetCell.Y*self.Grid.Columns)-self.Height/2,
									f32(targetCell.Z*self.Grid.Planes)-self.Length/2,
								),
								self.CellsToSize,
								self.CellsToSize,
								self.CellsToSize,
								rl.Green,
							)
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
