#include <core/rendering/resources/persistent_resources.h>

namespace lumen {

Error PersistentResources::initialize(drivers::DeviceDriverVulkan& r_dd)
{
    using enum Error;

    dd = &r_dd;
    frame_count = dd->frame_count;

    mesh_instance_buffers.resize(frame_count);
    transform_buffers.resize(frame_count);
    camera_buffers.resize(frame_count);
    
    for (uint32_t i = 0; i < frame_count; i++) {

        mesh_instance_buffers[i] = dd->buffer_create({
            .size = (VkDeviceSize)MAX_INSTANCES * sizeof(MeshInstance),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .device_local = false, .host_visible = true,
            .pool = dd->bar_pool(), .name = "mesh instances"
        });

        transform_buffers[i] = dd->buffer_create({
            .size = (VkDeviceSize)MAX_INSTANCES * sizeof(mat4),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .device_local = false, .host_visible = true,
            .pool = dd->bar_pool(), .name = "transforms"
        });

        camera_buffers[i] = dd->buffer_create({
            .size = sizeof(CameraUniform),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .device_local = false, .host_visible = true,
            .pool = dd->bar_pool(), .name = "camera"
        });

    }

    return Ok;
}

void PersistentResources::shutdown()
{
    for (uint32_t i = 0; i < frame_count; i++) {
        dd->buffer_free(mesh_instance_buffers[i]);
        dd->buffer_free(transform_buffers[i]);
        dd->buffer_free(camera_buffers[i]);
    }
}


}