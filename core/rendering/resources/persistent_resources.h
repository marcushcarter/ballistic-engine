#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/rendering/resources/scene_gpu.h>
#include <core/base/error.h>
#include <vector>

namespace lumen {

struct PersistentResources
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    uint32_t frame_count = 0;
    
    std::vector<drivers::DeviceDriverVulkan::Buffer> mesh_instance_buffers;
    std::vector<drivers::DeviceDriverVulkan::Buffer> transform_buffers;
    std::vector<drivers::DeviceDriverVulkan::Buffer> camera_buffers;
    uint32_t instance_count = 0;
    
    Error initialize(drivers::DeviceDriverVulkan& r_dd);
    void shutdown();
};

}