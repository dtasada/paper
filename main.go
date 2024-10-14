package main

import (
	"fmt"
	"strconv"

	rg "github.com/gen2brain/raylib-go/raygui"
	rl "github.com/gen2brain/raylib-go/raylib"

	e "github.com/dtasada/paper/engine"
	m "github.com/dtasada/paper/engine/math"
)

var particleCount int = 2

/* Movement keys */
func handleMovement(camera *rl.Camera3D) {
	mouseDelta := rl.GetMouseDelta()
	rl.CameraYaw(camera, -mouseDelta.X*e.Sensitivity, 0)
	rl.CameraPitch(camera, -mouseDelta.Y*e.Sensitivity, 0, 0, 0)

	var movementSpeed float32
	if rl.IsKeyDown(rl.KeyLeftShift) {
		movementSpeed = e.MovementSpeed * 2
	} else if rl.IsKeyDown(rl.KeyC) {
		movementSpeed = e.MovementSpeed / 2
	} else {
		movementSpeed = e.MovementSpeed
	}

	if rl.IsKeyDown(rl.KeyW) {
		rl.CameraMoveForward(camera, movementSpeed, 1)
	}
	if rl.IsKeyDown(rl.KeyA) {
		rl.CameraMoveRight(camera, -movementSpeed, 1)
	}
	if rl.IsKeyDown(rl.KeyS) {
		rl.CameraMoveForward(camera, -movementSpeed, 1)
	}
	if rl.IsKeyDown(rl.KeyD) {
		rl.CameraMoveRight(camera, movementSpeed, 1)
	}

	if rl.IsKeyDown(rl.KeySpace) {
		rl.CameraMoveUp(camera, movementSpeed)
	}
	if rl.IsKeyDown(rl.KeyLeftControl) {
		rl.CameraMoveUp(camera, -movementSpeed)
	}
}

func main() {
	fmt.Println("Initializing raylib...")

	/* Init raylib */
	rl.InitWindow(1280, 720, "paper")
	rl.SetExitKey(0)
	rl.DisableCursor()
	rl.SetTargetFPS(e.TargetFPS)

	/* Raylib flags */
	rl.SetTraceLogLevel(rl.LogWarning)
	rl.SetConfigFlags(rl.FlagMsaa4xHint) // Enable 4x MSAA if available
	rl.SetConfigFlags(rl.FlagWindowResizable)

	/* Raygui setup */
	e.Font = rl.LoadFont("./resources/fonts/CaskaydiaCoveNF.ttf")
	rg.SetFont(e.Font)
	rg.SetStyle(rg.DEFAULT, rg.TEXT_SIZE, 20)
	rg.SetStyle(rg.DEFAULT, rg.TEXT_COLOR_NORMAL, int64(rl.ColorToInt(rl.White)))

	/* Shader setup */
	lightShader := rl.LoadShader("./resources/shaders/lighting.vs", "./resources/shaders/lighting.fs")
	*lightShader.Locs = rl.GetShaderLocation(lightShader, "viewPos")
	ambientLoc := rl.GetShaderLocation(lightShader, "ambient")
	shaderValue := []float32{0.1, 0.1, 0.1, 1.0}
	rl.SetShaderValue(lightShader, ambientLoc, shaderValue, rl.ShaderUniformVec4)

	/* Logic setup (camera, container and particle slice) */
	camera := rl.Camera3D{
		Position:   rl.NewVector3(0, 0, 0),
		Target:     rl.NewVector3(-4, -4, -4),
		Up:         rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		Fovy:       120.0,
		Projection: rl.CameraPerspective,
	}

	container := e.NewContainer(
		rl.NewVector3(0, 0, 0),
		rl.NewVector3(100, 100, 100),
		5,
		lightShader,
	)

	light := e.NewLight(
		e.LightTypePoint,
		camera.Position,
		camera.Target,
		rl.Yellow,
		0.5,
		lightShader,
	)

	var particles []*e.Particle
	// particles := []*src.Particle{}

	/* Main loop */
	for !rl.WindowShouldClose() {
		{ /* Pre-render logic here */
			if rl.IsCursorHidden() {
				handleMovement(&camera)
				light.Position = camera.Position
				light.Target = camera.Target
			}

			rl.SetShaderValue(
				lightShader,
				*lightShader.Locs,
				[]float32{camera.Position.X, camera.Position.Y, camera.Position.Z},
				rl.ShaderUniformVec3,
			)

			if rl.IsKeyPressed(rl.KeyEscape) {
				if rl.IsCursorHidden() {
					rl.EnableCursor()
				} else {
					rl.DisableCursor()
				}
			}
		}

		{ /* Drawing */
			rl.BeginDrawing()
			rl.ClearBackground(rl.Black)

			{ /* 3D Rendering */
				rl.BeginMode3D(camera)

				for _, particle := range particles {
					particle.Update(&container, &particles)

					if rl.IsMouseButtonPressed(rl.MouseButtonRight) {
						particle.Vel = m.V3Random(10 * e.ForceMult)
					} else if rl.IsKeyReleased(rl.KeyP) {
						particle.Vel = m.V3Add(
							particle.Vel,
							m.V3MultVal(
								rl.Vector3Normalize(m.V3Sub(rl.Vector3Zero(), particle.Pos)),
								particle.Mass*e.ForceMult,
							),
						)
					}
				}

				light.Update()

				container.DrawBounds()

				rl.EndMode3D()
			} /* 3D Rendering */

			{ /* 2D Rendering */
				e.DrawText(fmt.Sprint(rl.GetFPS(), " FPS"), 12, 12+24*0, 20, rl.White)
				e.DrawText(fmt.Sprintf("X: %.2f; Y: %.2f; Z: %.2f", camera.Position.X, camera.Position.Y, camera.Position.Z), 12, 12+24*1, 20, rl.White)

				particleCount = m.Floor(
					rg.Slider(
						rl.NewRectangle(12+11*15, 60+24*1, 240, 20),
						"Particle count ",
						strconv.Itoa(particleCount),
						float32(particleCount),
						1,
						1000,
					),
				)

				e.Gravity = rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*2, 240, 20),
					"Gravity        ",
					fmt.Sprintf("%.2f g", e.Gravity*e.GravityMultiplier),
					e.Gravity*e.GravityMultiplier,
					0,
					10,
				) / e.GravityMultiplier

				e.TargetFPS = int32(rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*3, 240, 20),
					"Target FPS     ",
					strconv.Itoa(int(e.TargetFPS)),
					float32(e.TargetFPS),
					10,
					480,
				))
				rl.SetTargetFPS(e.TargetFPS)

				light.Intensity = rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*4, 240, 20),
					"Light intensity",
					fmt.Sprintf("%.2f", light.Intensity),
					light.Intensity,
					0.0,
					2.0,
				)

				e.ForceMult = rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*5, 240, 20),
					"Force Mult     ",
					fmt.Sprintf("%.2f", e.ForceMult),
					light.Intensity,
					0.0,
					2.0,
				)

				e.ShowCollisionGrid = rg.CheckBox(
					rl.NewRectangle(8, 60+24*6, 20, 20),
					"Collision grid ",
					e.ShowCollisionGrid,
				)

				e.ShowCollisionLines = rg.CheckBox(
					rl.NewRectangle(8, 60+24*7, 20, 20),
					"Collision lines",
					e.ShowCollisionLines,
				)

				e.ShowParticleCells = rg.CheckBox(
					rl.NewRectangle(8, 60+24*8, 20, 20),
					"Particle cells ",
					e.ShowParticleCells,
				)

				e.ShowCellModels = rg.CheckBox(
					rl.NewRectangle(8, 60+24*9, 20, 20),
					"Cell models    ",
					e.ShowCellModels,
				)

				e.ShowContainerWalls = rg.CheckBox(
					rl.NewRectangle(8, 60+24*10, 20, 20),
					"Container Walls",
					e.ShowContainerWalls,
				)

				e.ContainParticles = rg.CheckBox(
					rl.NewRectangle(8, 60+24*11, 20, 20),
					"Contain particles",
					e.ContainParticles,
				)
			} /* 2D Rendering */

			/* Generate or remove particles */
			if len(particles) > particleCount {
				rl.UnloadModel(particles[0].Model)
				particles = particles[1:]
			} else if len(particles) < particleCount {
				e.CreateParticle(&container, &particles, lightShader)
			}

			rl.EndDrawing()
		} /* Drawing */
	} /* Main loop */

	{ /* Cleanup */
		rl.UnloadShader(lightShader)
		rl.UnloadFont(e.Font)
		for _, particle := range particles {
			rl.UnloadModel(particle.Model)
		}
		rl.CloseWindow()
	}
}
