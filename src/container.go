package src

import (
	"fmt"

	"github.com/gen2brain/raylib-go/raylib"
)

type Grid struct {
	Columns int
	Rows    int
	Planes  int

	Content []Plane
}

func (self *Grid) PrintContent() {
	for i, z := range self.Content {
		fmt.Printf("i: %v [\n", i)
		for j, y := range z {
			fmt.Printf("j: %v \t[\n", j)
			for k, cell := range y {
				fmt.Printf("\t\tk: %v cell: %v,\n", k, cell)
			}
			fmt.Printf("\t],\n")
		}
		fmt.Printf("]\n")
	}
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
	Pos    Vector3
	Width  float32
	Height float32
	Length float32

	Bounds Bounds
	Grid   Grid
}

func NewContainer(pos, size, gridDensity Vector3) Container {
	width := size.X
	height := size.Y
	length := size.Z

	columns := int(gridDensity.X)
	rows := int(gridDensity.Y)
	planes := int(gridDensity.Z)

	return Container{
		Pos:    pos,
		Width:  width,
		Height: height,
		Length: length,

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
			[][][]GridCell{},
		},
	}
}

/* Find collision between a particle and any particle in its contiguous cells */
func (self *Container) FindCollisions(particle *Particle) {
	cell := Vector3Int{ // find particle's corresponding cell
		int(floorToZero((particle.Pos.X + self.Width/2.0) / f32(self.Grid.Columns))),
		int(floorToZero((particle.Pos.Y + self.Height/2.0) / f32(self.Grid.Rows))),
		int(floorToZero((particle.Pos.Z + self.Length/2.0) / f32(self.Grid.Planes))),
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
							/* rl.DrawCubeWires(
								rl.NewVector3(
									f32(targetCell.X*self.Grid.Rows)-self.Width/2,
									f32(targetCell.Y*self.Grid.Columns)-self.Height/2,
									f32(targetCell.Z*self.Grid.Planes)-self.Length/2,
								),
								particle.Radius,
								particle.Radius,
								particle.Radius,
								rl.Green,
							) */
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
				compensationDistance := pa.Radius + pb.Radius - rl.Vector3Distance(pa.Pos, pb.Pos)
				pa.Pos = rl.Vector3SubtractValue(pa.Pos, compensationDistance*pa.CollisionDamping*pb.CollisionDamping/2.0)
				pb.Pos = rl.Vector3AddValue(pb.Pos, compensationDistance*pb.CollisionDamping*pa.CollisionDamping/2.0)
			}
		}
	}
}
