#include "../include/Light.hpp"

#include <raylib.h>

#include <cstdio>
#include <format>

#include "../include/Engine.hpp"

Light::Light(LightType light_type, v3 position, v3 target, Color color, float intensity,
             Shader shader)
    : shader(shader),
      light_type(light_type),
      position(position),
      target(target),
      color(color),
      intensity(intensity) {
    if (light_count < MAX_LIGHTS_COUNT) {
        enabled = 1;
        std::string prefix = std::format("lights[{}]", light_count);

        enabled_loc = GetShaderLocation(shader, (prefix + ".enabled").c_str());
        type_loc = GetShaderLocation(shader, (prefix + ".type").c_str());
        pos_loc = GetShaderLocation(shader, (prefix + ".position").c_str());
        target_loc = GetShaderLocation(shader, (prefix + ".target").c_str());
        color_loc = GetShaderLocation(shader, (prefix + ".color").c_str());
        update();
        light_count++;
    }
}

void Light::update() {
    DrawSphere(position, 0.2, color);

    // Send to shader light enabled state and type
    SetShaderValue(shader, enabled_loc, &enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, type_loc, &light_type, SHADER_UNIFORM_INT);

    // Send to shader light position values
    SetShaderValue(shader, pos_loc, &position, SHADER_UNIFORM_VEC3);

    // Send to shader light target values
    SetShaderValue(shader, target_loc, &target, SHADER_UNIFORM_VEC3);

    // Send to shader light color values
    SetShaderValue(shader, color_loc,
                   (float[]){
                       color.r * intensity / 255,
                       color.g * intensity / 255,
                       color.b * intensity / 255,
                       color.a * intensity / 255,
                   },
                   SHADER_UNIFORM_VEC4);
}

void Light::SetPosition(v3 position) { this->position = position; }
void Light::SetTarget(v3 target) { this->target = target; }
