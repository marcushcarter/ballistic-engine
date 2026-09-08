#pragma once
#include <core/world/camera.h>
#include <core/base/error.h>

namespace lumen {

struct World
{
    Camera default_camera;
    Camera* active_camera = &default_camera;
    
    Error load();
    void unload();
};

}