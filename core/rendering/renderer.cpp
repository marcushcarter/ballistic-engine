#include <core/rendering/renderer.h>
#include <core/scene/scenes.h>
#include <core/assets/asset_common.h>
#include <core/io/embedded_resource.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <cmath>

namespace lumen {

using namespace glm;

void Renderer::_create_hiz(uint32_t p_width, uint32_t p_height)
{
    uint32_t hw = (p_width + 1) / 2;
    uint32_t hh = (p_height + 1) / 2;
    uint32_t mips = 1, m = std::max(hw, hh);
    while (m > 1) { m >>= 1; mips++; }

    drivers::DeviceDriverVulkan::ImageCreateInfo ci{};
    ci.format = VK_FORMAT_R16_SFLOAT;
    ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    ci.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    ci.mip_levels = mips;
    ci.sizing = drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::Fixed;
    ci.fixed_width = hw;
    ci.fixed_height = hh;
    ci.name = "hiz";
    hiz = dd->image_create_dedicated(ci, { hw, hh });

    VkClearColorValue far_clear{};
    far_clear.float32[0] = 0.0f;
    dd->image_clear(hiz, far_clear);
}

void Renderer::_destroy_hiz()
{
    if (hiz.image) dd->image_free(hiz);
    hiz = {};
}

Error Renderer::initialize(drivers::DeviceDriverVulkan& r_dd)
{
    using enum Error;

    dd = &r_dd;
    frame_count = dd->frame_count;
    current_frame = 0;

    in_flight_fences.resize(frame_count);
    image_available_semaphores.resize(frame_count);
    command_pools.resize(frame_count);
    command_buffers.resize(frame_count);

    uint32_t graphics_family = dd->cd->graphics_queue_family;

    for (uint32_t i = 0; i < frame_count; i++) {
        in_flight_fences[i] = dd->fence_create(true);
        image_available_semaphores[i] = dd->semaphore_create();
        command_pools[i] = dd->command_pool_create(graphics_family);
        command_buffers[i] = dd->command_buffer_create(command_pools[i]);
    }

    Error err = textures.initialize(r_dd);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    err = geometry.initialize(r_dd);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);

    err = graph.initialize(r_dd, frame_count);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    graph.declare_image_format("Backbuffer", dd->swapchain.format);
    
    err = frame.initialize(r_dd);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);

    set_size(1, 1);
    pending_width = width;
    pending_height = height;

    return Ok;
}

void Renderer::shutdown()
{
    unload();
    
    graph.shutdown();
    frame.shutdown();
    
    _destroy_hiz();

    for (uint32_t i = 0; i < frame_count; i++) {
        dd->fence_free(in_flight_fences[i]);
        dd->semaphore_free(image_available_semaphores[i]);
        dd->command_pool_free(command_pools[i]);
    }
}

Error Renderer::load(const std::filesystem::path& p_content_dir)
{
    using enum Error;
    
    Error err = geometry.allocate();
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);

    std::error_code ec;
    if (!std::filesystem::exists(p_content_dir, ec)) return Ok;

    for (auto it = std::filesystem::recursive_directory_iterator(p_content_dir, ec); !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
        if (!it->is_regular_file(ec)) continue;
        const std::filesystem::path& path = it->path();
        if (path.extension() != ".bin") continue;
        LAssetHeader ah{};
        if (!read_asset_header(path, ah)) continue;
        switch (ah.type) {
            case AssetType::Texture:
                textures.load(ah.guid, path);
                break;
            case AssetType::Mesh:
                geometry.load(ah.guid, path);
                break;
            default:
                break;
        }
    }

    return Ok;
}

void Renderer::unload()
{
    textures.clear();
    geometry.free();
}

Error Renderer::set_size(uint32_t p_width, uint32_t p_height)
{
    using enum Error;

    if (p_width == 0 || p_height == 0) return Ok;
    if (p_width == width && p_height == height) return Ok;
    width = p_width;
    height = p_height;
    resize_epoch++;
    
    dd->device_wait_idle();

    Error err = graph.set_size(p_width, p_height);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);

    _destroy_hiz();
    _create_hiz(p_width, p_height);

    return Ok;
}

void Renderer::request_size(uint32_t p_width, uint32_t p_height)
{
    pending_width = p_width;
    pending_height = p_height;
}

Error Renderer::apply_pending_size()
{
    if (pending_width == 0 || pending_height == 0) return Error::Ok;
    return set_size(pending_width, pending_height);
}

static glm::mat4 perspective_reverse_z(float fov_y, float aspect, float near_z, float far_z)
{
    const float f = 1.0f / std::tan(fov_y * 0.5f);
    glm::mat4 m(0.0f);
    m[0][0] = f / aspect;
    m[1][1] = f;
    // reverse-Z, [0,1] depth: near maps to 1, far to 0
    m[2][2] = near_z / (far_z - near_z);
    m[2][3] = -1.0f;
    m[3][2] = (far_z * near_z) / (far_z - near_z);
    return m;  // column-major glm
}

static void extract_frustum_planes(const glm::mat4& vp, glm::vec4 out[6])
{
    auto row = [&](int i) { return glm::vec4(vp[0][i], vp[1][i], vp[2][i], vp[3][i]); };
    const glm::vec4 r0 = row(0), r1 = row(1), r2 = row(2), r3 = row(3);
    out[0] = r3 + r0;  // left
    out[1] = r3 - r0;  // right
    out[2] = r3 + r1;  // bottom
    out[3] = r3 - r1;  // top
    out[4] = r3 + r2;  // near
    out[5] = r3 - r2;  // far
    for (int i = 0; i < 6; i++) out[i] /= glm::length(glm::vec3(out[i])); // normalize
}

void Renderer::_frame_prepare(const Scene&)
{
    for (uint32_t i = 0; i < (uint32_t)geometry.meshes.size(); i++) {
        for (int j=0; j<10; j++) {
            frame.instances_scratch.push_back(Instance{ i, (uint32_t)frame.transforms_scratch.size(), 0, 0 });
            frame.transforms_scratch.push_back(Transform{ mat4(1.0f), mat4(1.0f) });
            frame.cluster_ref_capacity += geometry.meshes[i].cluster_count;
        }
    }

    frame.instance_count = (uint32_t)frame.instances_scratch.size();

    if (frame.instance_count != 0) {
        drivers::DeviceDriverVulkan::Buffer& ib = frame.instance_buffers[current_frame];
        dd->buffer_update(ib, frame.instances_scratch.data(), (VkDeviceSize)frame.instance_count * sizeof(Instance));
        dd->buffer_flush(ib, 0, (VkDeviceSize)frame.instance_count * sizeof(Instance));

        drivers::DeviceDriverVulkan::Buffer& tb = frame.transform_buffers[current_frame];
        dd->buffer_update(tb, frame.transforms_scratch.data(), (VkDeviceSize)frame.instance_count * sizeof(Transform));
        dd->buffer_flush(tb, 0, (VkDeviceSize)frame.instance_count * sizeof(Transform));
    }
    
    {
    const float aspect = height ? (float)width / (float)height : 1.0f;
    const float fov_y = radians(60.0f);
    const float near_z = 2.0f, far_z = 5.0f;
    // const float near_z = 0.1f, far_z = 1000.0f;

    static const auto start = std::chrono::high_resolution_clock::now();
    const float t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count();
    const float radius = 3.0f, angle = t * radians(45.0f);
    const vec3 eye(radius * cos(angle), 1.0f, radius * sin(angle));

    CameraUniform cam{};
    cam.view = lookAt(eye, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    cam.proj = perspective_reverse_z(fov_y, aspect, near_z, far_z);
    cam.view_proj = cam.proj * cam.view;
    cam.inv_view = inverse(cam.view);
    cam.inv_proj = inverse(cam.proj);
    cam.inv_view_proj = inverse(cam.view_proj);
    cam.position = vec4(eye, 1.0f);
    extract_frustum_planes(cam.view_proj, cam.frustum_planes);
    cam.near_z = near_z;
    cam.far_z = far_z;
    cam.fov_y = fov_y;
    cam.aspect = aspect;

    drivers::DeviceDriverVulkan::Buffer& cb = frame.camera_buffers[current_frame];
    dd->buffer_update(cb, &cam, sizeof(cam));
    dd->buffer_flush(cb, 0, sizeof(cam));
    }
}

Error Renderer::begin_frame(const Scene& p_scene)
{
    using enum Error;
    
    auto& sc = dd->swapchain;

    Error err = dd->fence_wait(in_flight_fences[current_frame]);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    err = dd->fence_reset(in_flight_fences[current_frame]);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    err = dd->swapchain_acquire_next_image(image_available_semaphores[current_frame]);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);

    frame.update(p_scene, current_frame);
    _frame_prepare(p_scene);

    graph.begin(current_frame);
    graph.import_image("Backbuffer", &sc.images[sc.image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);
    graph.import_image("HiZ", &hiz, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    // graph.import_image("HiZ", &hiz, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_SAMPLED_READ_BIT);

    graph.import_buffer("Camera", &frame.camera_buffers[current_frame], VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);
    graph.import_buffer("Geometry", &geometry.address_buffer, VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);
    graph.import_buffer("Instances", &frame.instance_buffers[current_frame], VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);
    graph.import_buffer("Transforms", &frame.transform_buffers[current_frame], VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, 0);

    return Ok;
}

void Renderer::compile()
{
    graph.compile();
}

Error Renderer::record()
{
    using enum Error;

    Error err = dd->command_pool_reset(command_pools[current_frame]);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    err = dd->command_buffer_begin(command_buffers[current_frame], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);
    
    VkCommandBuffer cmd = command_buffers[current_frame];

    dd->command_bind_graphics_uniform_sets(cmd, { dd->bindless_heap.set });
    dd->command_bind_compute_uniform_sets(cmd, { dd->bindless_heap.set });

    graph.execute(cmd);
    
    err = dd->command_buffer_end(command_buffers[current_frame]);
    LUMEN_ERR_FAIL_COND_V(err != Ok, err);

    return Ok;
}

Error Renderer::end_frame()
{
    using enum Error;
    auto& sc = dd->swapchain;

    VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &image_available_semaphores[current_frame];
    submit_info.pWaitDstStageMask = &wait_stage;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &sc.present_semaphores[sc.image_index];

    VkQueue graphics_queue = dd->queue_families[dd->cd->graphics_queue_family][0].queue;
    VkResult result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);
    LUMEN_ERR_FAIL_COND_V_MSG(result != VK_SUCCESS, Failed, "Failed to submit Vulkan queue");

    VkPresentInfoKHR present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &sc.present_semaphores[sc.image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &sc.swapchain;
    present_info.pImageIndices = &sc.image_index;

    VkQueue present_queue = dd->queue_families[dd->cd->present_queue_family][0].queue;
    result = vkQueuePresentKHR(present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        dd->swapchain.surface->needs_resize = true;
    } else {
        LUMEN_ERR_FAIL_COND_V_MSG(result != VK_SUCCESS, Failed, "Failed to present Vulkan queue");
    }

    current_frame = (current_frame + 1) % frame_count;
    frame_number++;

    return Ok;
}

RenderContext Renderer::make_context()
{
    RenderContext ctx{};
    ctx.dd = dd;
    ctx.graph = &graph;
    ctx.textures = &textures;
    ctx.geometry = &geometry;
    ctx.frame = &frame;
    return ctx;
}

}