#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/rendering/scene_gpu.h>
#include <core/scene/scenes.h>
#include <core/base/error.h>
#include <vector>
#include <glm/glm.hpp>

namespace lumen {

struct FrameData
{    
    std::vector<Instance> instances_scratch;
    std::vector<Transform> transforms_scratch;
    uint32_t instance_count = 0;
    uint32_t cluster_ref_capacity = 0;

    CameraUniform camera{};

    void reset();
};

}