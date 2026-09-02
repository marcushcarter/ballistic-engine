#pragma once
#include <core/assets/guid.h>
#include <core/base/error.h>
#include <vector>

namespace lumen {

struct Scene
{

};

struct Scenes
{
    std::vector<Scene> scenes;
    Scene* active = nullptr;

    Error load() { return Error::Ok; }
    void unload() {}
};

}