#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/rendering/render_graph.h>
#include <core/rendering/resources/texture_cache.h>
#include <core/rendering/resources/geometry_pool.h>
#include <core/base/error.h>

namespace lumen {

struct Renderer
{
    drivers::DeviceDriverVulkan* dd = nullptr;

    RenderGraph graph;
    
    TextureCache textures;
    GeometryPool geometry;

    uint32_t width = 0;
    uint32_t height = 0;
    uint64_t resize_epoch = 0;
    
    uint32_t pending_width = 0;
    uint32_t pending_height = 0;
    
    uint32_t frame_count = 1;
    uint32_t current_frame = 0;
    uint64_t frame_number = 0;

    std::vector<VkSemaphore> image_available_semaphores;
    std::vector<VkFence> in_flight_fences;
    std::vector<drivers::DeviceDriverVulkan::CommandPool> command_pools;
    std::vector<VkCommandBuffer> command_buffers;

    Error initialize(drivers::DeviceDriverVulkan& r_dd);
    void shutdown();

    Error load(const std::filesystem::path& p_content_dir);
    void unload();

    void request_size(uint32_t p_width, uint32_t p_height);
    Error apply_pending_size();
    Error set_size(uint32_t p_width, uint32_t p_height);

    Error begin_frame();
    void compile();
    Error record();
    Error end_frame();
};

}