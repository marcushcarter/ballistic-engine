#include <core/rendering/frame_data.h>

namespace lumen {

Error FrameData::initialize(drivers::DeviceDriverVulkan& r_dd)
{
    using enum Error;

    dd = &r_dd;
    frame_count = dd->frame_count;

    instance_buffers.resize(frame_count);
    transform_buffers.resize(frame_count);
    camera_buffers.resize(frame_count);
    
    for (uint32_t i = 0; i < frame_count; i++) {

        instance_buffers[i] = dd->buffer_create({
            .size = (VkDeviceSize)MAX_INSTANCES * sizeof(Instance),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .device_local = false,
            .host_visible = true,
            .pool = dd->bar_pool(),
            .name = "instances"
        });

        transform_buffers[i] = dd->buffer_create({
            .size = (VkDeviceSize)MAX_INSTANCES * sizeof(Transform),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .device_local = false,
            .host_visible = true,
            .pool = dd->bar_pool(),
            .name = "transforms"
        });

        camera_buffers[i] = dd->buffer_create({
            .size = sizeof(CameraUniform),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .device_local = false,
            .host_visible = true,
            .pool = dd->bar_pool(),
            .name = "camera"
        });

    }

    return Ok;
}

void FrameData::shutdown()
{
    for (uint32_t i = 0; i < frame_count; i++) {
        dd->buffer_free(instance_buffers[i]);
        dd->buffer_free(transform_buffers[i]);
        dd->buffer_free(camera_buffers[i]);
    }
}

void FrameData::update(const Scene& p_scene, uint32_t p_frame_index)
{
    (void)p_scene;
    (void)p_frame_index;

    instances_scratch.clear();
    transforms_scratch.clear();

    // Camera.
    {
        
    }
}

}