package main

import (
	"fmt"
	"strconv"

	rg "github.com/gen2brain/raylib-go/raygui"
	rl "github.com/gen2brain/raylib-go/raylib"

	"github.com/dtasada/paper/src"
)

var particleCount int = 100

/* Movement keys */
func handleMovement(camera *rl.Camera3D) {
	mouseDelta := rl.GetMouseDelta()
	rl.CameraYaw(camera, -mouseDelta.X*src.Sensitivity, 0)
	rl.CameraPitch(camera, -mouseDelta.Y*src.Sensitivity, 0, 0, 0)

	var movementSpeed float32
	if rl.IsKeyDown(rl.KeyLeftShift) {
		movementSpeed = src.MovementSpeed * 2
	} else if rl.IsKeyDown(rl.KeyC) {
		movementSpeed = src.MovementSpeed / 2
	} else {
		movementSpeed = src.MovementSpeed
	}

	if rl.IsKeyDown(rl.KeyW) {
		rl.CameraMoveForward(camera, movementSpeed, 0)
	}
	if rl.IsKeyDown(rl.KeyA) {
		rl.CameraMoveRight(camera, -movementSpeed, 0)
	}
	if rl.IsKeyDown(rl.KeyS) {
		rl.CameraMoveForward(camera, -movementSpeed, 0)
	}
	if rl.IsKeyDown(rl.KeyD) {
		rl.CameraMoveRight(camera, movementSpeed, 0)
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

	screenWidth := 1280
	screenHeight := 720
	// screenCenter := rl.NewVector2(float32(rl.GetScreenWidth())/2, float32(rl.GetScreenHeight())/2)

	/* Init raylib */
	rl.InitWindow(int32(screenWidth), int32(screenHeight), "paper")
	rl.SetExitKey(0)
	rl.DisableCursor()
	rl.SetTargetFPS(src.TargetFPS)

	/* Raylib flags */
	rl.SetTraceLogLevel(rl.LogWarning)
	rl.SetConfigFlags(rl.FlagMsaa4xHint) // Enable 4x MSAA if available
	rl.SetConfigFlags(rl.FlagWindowResizable)

	/* Raygui setup */
	src.Caskaydia = rl.LoadFont("./resources/fonts/CaskaydiaCoveNF.ttf")
	rg.SetFont(src.Caskaydia)
	rg.SetStyle(rg.DEFAULT, rg.TEXT_SIZE, 20)
	rg.SetStyle(rg.DEFAULT, rg.TEXT_COLOR_NORMAL, int64(rl.ColorToInt(rl.White)))

	/* Shader setup */
	lightShader := rl.LoadShader("./resources/shaders/lighting.vs", "./resources/shaders/lighting.fs")
	*lightShader.Locs = rl.GetShaderLocation(lightShader, "viewPos")
	ambientLoc := rl.GetShaderLocation(lightShader, "ambient")
	shaderValue := []float32{0.1, 0.1, 0.1, 1.0}
	rl.SetShaderValue(lightShader, ambientLoc, shaderValue, rl.ShaderUniformVec4)

	/* Logic setup (camera, container and particle arraylist) */
	camera := rl.Camera3D{
		Position:   rl.NewVector3(0, 0, 0),
		Target:     rl.NewVector3(-4, -4, -4),
		Up:         rl.NewVector3(0, 1, 0), // Asserts Y to be the vertical axis
		Fovy:       120.0,
		Projection: rl.CameraPerspective,
	}

	container := src.NewContainer(
		rl.NewVector3(0, 0, 0),
		rl.NewVector3(100, 100, 100),
		5,
		lightShader,
	)

	light := src.NewLight(src.LightTypePoint, camera.Position, camera.Target, rl.Yellow, 0.5, lightShader)

	particles := []*src.Particle{}

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
						particle.Vel = src.Vector3Random(10)
					}
				}

				light.Update()

				container.DrawBounds()

				rl.EndMode3D()
			} /* 3D Rendering */

			{ /* 2D Rendering */
				src.DrawText(fmt.Sprint(rl.GetFPS(), " FPS"), 12, 12+24*0, 20, rl.White)
				src.DrawText(fmt.Sprintf("X: %.2f; Y: %.2f; Z: %.2f", camera.Position.X, camera.Position.Y, camera.Position.Z), 12, 12+24*1, 20, rl.White)

				particleCount = src.Floor(
					rg.Slider(
						rl.NewRectangle(12+11*15, 60+24*1, 240, 20),
						"Particle count ",
						strconv.Itoa(particleCount),
						float32(particleCount),
						1,
						200,
					),
				)

				src.Gravity = rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*2, 240, 20),
					"Gravity        ",
					fmt.Sprintf("%.2f g", src.Gravity*src.GravityMultiplier),
					src.Gravity*src.GravityMultiplier,
					0,
					10,
				) / src.GravityMultiplier

				src.TargetFPS = int32(rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*3, 240, 20),
					"Target FPS     ",
					strconv.Itoa(int(src.TargetFPS)),
					float32(src.TargetFPS),
					10,
					480,
				))
				rl.SetTargetFPS(src.TargetFPS)

				light.Intensity = rg.Slider(
					rl.NewRectangle(12+11*15, 60+24*4, 240, 20),
					"Light intensity",
					fmt.Sprintf("%.2f", light.Intensity),
					light.Intensity,
					0.0,
					2.0,
				)

				src.ShowCollisionGrid = rg.CheckBox(
					rl.NewRectangle(8, 60+24*5, 20, 20),
					"Collision grid ",
					src.ShowCollisionGrid,
				)

				src.ShowCollisionLines = rg.CheckBox(
					rl.NewRectangle(8, 60+24*6, 20, 20),
					"Collision lines",
					src.ShowCollisionLines,
				)

				src.ShowParticleCells = rg.CheckBox(
					rl.NewRectangle(8, 60+24*7, 20, 20),
					"Particle cells ",
					src.ShowParticleCells,
				)

				src.ShowCellModels = rg.CheckBox(
					rl.NewRectangle(8, 60+24*8, 20, 20),
					"Cell models    ",
					src.ShowCellModels,
				)
			} /* 2D Rendering */

			/* Generate or remove particles */
			if len(particles) > particleCount {
				rl.UnloadModel(particles[0].Model)
				particles = particles[1:]
			} else if len(particles) < particleCount {
				src.CreateParticle(&container, &particles, lightShader)
			}

			rl.EndDrawing()
		} /* Drawing */
	} /* Main loop */

	{ /* Cleanup */
		rl.UnloadShader(lightShader)
		rl.UnloadFont(src.Caskaydia)
		for _, particle := range particles {
			rl.UnloadModel(particle.Model)
		}
		rl.CloseWindow()
	}
}
