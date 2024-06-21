package main

import (
	"fmt"
	"math"
	"slices"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type GridCell []*Particle
type tuple[A any, B any] struct {
	a A
	b B
}

const GRAVITY float32 = 9.81

func GetSign(number float32) float32 {
	if number > 0 {
		return 1
	} else if number < 0 {
		return -1
	} else {
		return 0
	}
}

type Grid struct {
	Width   float32
	Height  float32
	Content [][]GridCell
}

func NewGrid() Grid {
	width := float32(20)
	height := float32(20)

	return Grid{
		Width:   width,
		Height:  height,
		Content: [][]GridCell{},
	}
}

/* Finds collisions with adjacent cells */
func (self *Grid) FindCollisions() {
	/* x and y start at one to skip topmost and bottommost rows, and leftmost and rightmost columns */
	for x := 1; float32(x) <= self.Width; x++ {
		for y := 1; float32(y) <= self.Height; y++ {
			for dx := -1; dx <= 1; dx++ {
				for dy := -1; dy <= 1; dy++ {
					self.SolveCollision(x, y, x+dx, y+dy)
				}
			}
		}
	}
}

func (self *Grid) SolveCollision(cellAX, cellAY, cellBX, cellBY int) {
	fmt.Println("content:", self.Content)
	fmt.Println("len content:", len(self.Content))
	fmt.Println("cellAY:", cellAY)
	cellA := self.Content[cellAY][cellAX]
	cellB := self.Content[cellBY][cellBX]

	for _, a := range cellA {
		for _, b := range cellB {
			if a == b { // check pointer equality
				if !rl.CheckCollisionCircles(a.Pos, a.Radius, b.Pos, b.Radius) {
					compensationLength := a.Radius + b.Radius - rl.Vector2Distance(a.Pos, b.Pos)
					a.Pos = rl.NewVector2(
						a.Pos.X-compensationLength/2,
						a.Pos.Y-compensationLength/2,
					)
				}
			}
		}
	}
}

type Particle struct {
	Pos              rl.Vector2
	Vel              rl.Vector2
	Radius           float32
	CollisionDamping float32
}

func NewParticle(startPos, vel rl.Vector2, radius float32) Particle {
	return Particle{
		Pos:              startPos,
		Vel:              vel,
		Radius:           radius,
		CollisionDamping: 0.8,
	}
}

func (self *Particle) Collision(bounds *rl.Rectangle) {
	boundsSize := rl.NewVector2(bounds.X+bounds.Width, bounds.Y+bounds.Height)

	if self.Pos.X > boundsSize.X {
		self.Pos.X = boundsSize.X * GetSign(self.Pos.X)
		self.Vel.X *= -1.0 * self.CollisionDamping
	}

	if self.Pos.Y > boundsSize.Y {
		self.Pos.Y = boundsSize.Y * GetSign(self.Pos.Y)
		self.Vel.Y *= -1.0 * self.CollisionDamping
	}
}

func (self *Particle) InCell(grid *Grid, cell rl.Vector2) bool {
	if self.Pos.X > grid.Width*cell.X &&
		self.Pos.X < grid.Width*cell.X+grid.Width &&
		self.Pos.Y > grid.Width*cell.Y+grid.Height &&
		self.Pos.Y < grid.Width*cell.Y {
		return true
	} else {
		return false
	}
}

func (self *Particle) Update(bounds *rl.Rectangle, grid *Grid) {
	self.Vel.Y += GRAVITY

	self.Pos = rl.Vector2Add(self.Pos, self.Vel)
	self.Collision(bounds)

	for _, row := range grid.Content {
		for _, cell := range row {
			for k, particle := range cell {
				if particle == self {
					y := int(math.Floor(float64(self.Pos.Y / grid.Height)))
					x := int(math.Floor(float64(self.Pos.X / grid.Width)))
					grid.Content[y][x] = append(grid.Content[y][x], cell[k])
					slices.Delete(cell, k, k+1) // shitty go way of removing from slice by index
				}
			}
		}

		rl.DrawCircle(
			int32(self.Pos.X),
			int32(self.Pos.Y),
			self.Radius,
			rl.SkyBlue,
		)
	}
}
