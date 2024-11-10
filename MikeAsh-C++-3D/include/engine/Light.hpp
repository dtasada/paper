#ifndef LIGHT_HEADER
#define LIGHT_HEADER

#include <raylib.h>

#include "Engine.hpp"

#define MAX_LIGHTS_COUNT 4

enum LightType { DIRECTIONAL, POINT };

class Light {
   private:
    Shader shader;
    LightType light_type;
    v3 position;
    v3 target;
    Color color;
    float intensity;
    int enabled;
    // shader locations
    int enabled_loc;
    int type_loc;
    int pos_loc;
    int target_loc;
    int color_loc;

    int light_count = 4;

   public:
    Light(LightType light_type, v3 position, v3 target, Color color, float intensity,
          Shader shader);
    void update();
    void SetPosition(v3 position);
    void SetTarget(v3 target);
};

#endif
