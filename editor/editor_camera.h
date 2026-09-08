#pragma once
#include <core/world/camera.h>
#include <glm/glm.hpp>

namespace lumen {

using namespace glm;

struct EditorCamera
{
    Camera camera;

    vec3 target = vec3(0.0f);
    float radius = 6.0f;
    float height = 2.0f;
    float orbit_speed = 0.4f; // rad/sec
    float angle = 0.0f;

    void update(float p_dt);
};

}
