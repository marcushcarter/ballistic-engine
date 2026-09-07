#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/rendering/scene_gpu.h>
#include <core/scene/scenes.h>
#include <core/base/error.h>
#include <vector>

namespace lumen {

struct FrameData
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    uint32_t frame_count = 0;
    
    std::vector<Instance> instances_scratch;
    std::vector<Transform> transforms_scratch;
    uint32_t instance_count = 0;
    uint32_t cluster_ref_capacity = 0;
    
    std::vector<drivers::DeviceDriverVulkan::Buffer> instance_buffers;
    std::vector<drivers::DeviceDriverVulkan::Buffer> transform_buffers;
    std::vector<drivers::DeviceDriverVulkan::Buffer> camera_buffers;
    
    Error initialize(drivers::DeviceDriverVulkan& r_dd);
    void shutdown();

    void update(const Scene& p_scene, uint32_t p_frame_index);
};

}