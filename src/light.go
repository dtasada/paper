package src

import (
	"fmt"
	"unsafe"

	rl "github.com/gen2brain/raylib-go/raylib"
)

type LightType int32

var fstr = fmt.Sprintf

const (
	LightTypeDirectional LightType = iota
	LightTypePoint
)

type Light struct {
	Shader    rl.Shader
	LightType LightType
	Position  Vector3
	Target    Vector3
	Color     rl.Color
	Intensity float32
	Enabled   int32
	// shader locations
	EnabledLoc int32
	TypeLoc    int32
	PosLoc     int32
	TargetLoc  int32
	ColorLoc   int32
}

const maxLightsCount = 4

var lightCount = 0

func NewLight(
	lightType LightType,
	position, target Vector3,
	color rl.Color,
	intensity float32,
	shader rl.Shader,
) Light {
	light := Light{
		Shader: shader,
	}
	if lightCount < maxLightsCount {
		light.Enabled = 1
		light.LightType = lightType
		light.Position = position
		light.Target = target
		light.Color = color
		light.Intensity = intensity
		light.EnabledLoc = rl.GetShaderLocation(shader, fstr("lights[%d].enabled", lightCount))
		light.TypeLoc = rl.GetShaderLocation(shader, fstr("lights[%d].type", lightCount))
		light.PosLoc = rl.GetShaderLocation(shader, fstr("lights[%d].position", lightCount))
		light.TargetLoc = rl.GetShaderLocation(shader, fstr("lights[%d].target", lightCount))
		light.ColorLoc = rl.GetShaderLocation(shader, fstr("lights[%d].color", lightCount))
		light.Update()
		lightCount++
	}
	return light
}

func (self *Light) Update() {
	rl.DrawSphere(self.Position, 0.2, self.Color)

	// Send to shader light enabled state and type
	rl.SetShaderValue(self.Shader, self.EnabledLoc, unsafe.Slice((*float32)(unsafe.Pointer(&self.Enabled)), 4), rl.ShaderUniformInt)
	rl.SetShaderValue(self.Shader, self.TypeLoc, unsafe.Slice((*float32)(unsafe.Pointer(&self.LightType)), 4), rl.ShaderUniformInt)

	// Send to shader light position values
	rl.SetShaderValue(self.Shader, self.PosLoc, []float32{self.Position.X, self.Position.Y, self.Position.Z}, rl.ShaderUniformVec3)

	// Send to shader light target values
	rl.SetShaderValue(self.Shader, self.TargetLoc, []float32{self.Target.X, self.Target.Y, self.Target.Z}, rl.ShaderUniformVec3)

	// Send to shader light color values
	rl.SetShaderValue(
		self.Shader, self.ColorLoc,
		[]float32{
			float32(self.Color.R) * self.Intensity / 255,
			float32(self.Color.G) * self.Intensity / 255,
			float32(self.Color.B) * self.Intensity / 255,
			float32(self.Color.A) * self.Intensity / 255,
		},
		rl.ShaderUniformVec4,
	)
}
