#include <core/rendering/features/geometry_feature.h>
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/io/embedded_resource.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE   // Vulkan depth is [0,1], not GL's [-1,1]
#include <glm/glm.hpp>                 // vec3, mat4, radians
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>

namespace ballistic {

Error GeometryFeature::create_resources()
{
    geometry_pass.name = "Deferred";
    geometry_pass.category = "Geometry";
    geometry_pass.setup = [](RenderGraph::Builder& b) {
        
        drivers::DeviceDriverVulkan::ImageCreateInfo depth_ci{};
        depth_ci.format = VK_FORMAT_D32_SFLOAT;
        depth_ci.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        depth_ci.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        b.create_image("G_Depth", depth_ci);

        // drivers::DeviceDriverVulkan::ImageCreateInfo albedo_image_ci{};
        // albedo_image_ci.name = "G_Albedo";
        // albedo_image_ci.format = VK_FORMAT_R8G8B8A8_SRGB;
        // albedo_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        // b.create_image("G_Albedo", albedo_image_ci);

        // drivers::DeviceDriverVulkan::ImageCreateInfo normal_image_ci{};
        // normal_image_ci.name = "G_Normal";
        // normal_image_ci.format = VK_FORMAT_R16G16_UNORM;
        // normal_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        // b.create_image("G_Normal", normal_image_ci);

        // drivers::DeviceDriverVulkan::ImageCreateInfo rough_met_image_ci{};
        // rough_met_image_ci.name = "G_Rough_Metal";
        // rough_met_image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        // rough_met_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        // b.create_image("G_Rough_Metal", rough_met_image_ci);

        // drivers::DeviceDriverVulkan::ImageCreateInfo motion_image_ci{};
        // motion_image_ci.name = "G_Motion";
        // motion_image_ci.format = VK_FORMAT_R16G16_SFLOAT;
        // motion_image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        // b.create_image("G_Motion", motion_image_ci);

        // b.color_attachment("G_Albedo", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        // b.color_attachment("G_Normal", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        // b.color_attachment("G_Rough_Metal", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        // b.color_attachment("G_Motion", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        // b.depth_attachment_read("G_Depth", VK_ATTACHMENT_LOAD_OP_LOAD);
        
        b.color_attachment("Out_Color", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 0.0f, 0.0f, 1.0f } });
        b.depth_attachment("G_Depth", VK_ATTACHMENT_LOAD_OP_CLEAR, [] { VkClearValue v{}; v.depthStencil = { 1.0f, 0 }; return v; }());
    };
    geometry_pass.execute = [this](RenderGraph::CommandList& cl) {

        auto* bb = cl.graph->image("Out_Color");
        cl.dd->command_render_set_viewport(cl.cmd, {{{0,0},bb->extent}});
        cl.dd->command_render_set_scissor(cl.cmd, {{{0,0},bb->extent}});

        static const auto start = std::chrono::high_resolution_clock::now();
        const auto now = std::chrono::high_resolution_clock::now();
        const float t = std::chrono::duration<float>(now - start).count();

        const float radius = 3.0f;
        const float angle = t * glm::radians(45.0f);
        const glm::vec3 eye = glm::vec3(radius * std::cos(angle), 1.0f, radius * std::sin(angle));

        const float aspect = bb->extent.width / (float)bb->extent.height;
        glm::mat4 view = glm::lookAt(eye, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(60.0f), aspect, 2.0f, 5.0f);
        glm::mat4 view_proj = proj * view;
        
        cl.dd->command_bind_pipeline(cl.cmd, triangle_pipeline);
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(view_proj), &view_proj);
        cl.draw("triangle", 3);
    };

    return Error::Ok;
};

Error GeometryFeature::create_pipelines()
{
    VkRenderPass rp = ctx->graph->acquire_render_pass(geometry_pass);

    EmbeddedResource::Blob tri_vert_blob = EmbeddedResource::load(L"SHADERS_TRIANGLE_VERT");
    EmbeddedResource::Blob tri_frag_blob = EmbeddedResource::load(L"SHADERS_TRIANGLE_FRAG");
    VkShaderModule tri_vs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Vertex, .glsl = (const char*)tri_vert_blob.data, .glsl_size = tri_vert_blob.size, .name = "triangle_vs" });
    VkShaderModule tri_fs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Fragment, .glsl = (const char*)tri_frag_blob.data, .glsl_size = tri_frag_blob.size, .name = "triangle_fs" });

    drivers::DeviceDriverVulkan::GraphicsPipelineCreateInfo pipeline_ci{};
    pipeline_ci.vertex_shader = tri_vs;
    pipeline_ci.fragment_shader = tri_fs;
    pipeline_ci.render_pass = rp;
    pipeline_ci.cull_mode = VK_CULL_MODE_NONE;
    pipeline_ci.depth_test = true;
    pipeline_ci.depth_write = true;
    pipeline_ci.depth_compare = VK_COMPARE_OP_LESS;
    pipeline_ci.name = "triangle_pipeline";
    triangle_pipeline = ctx->dd->graphics_pipeline_create(pipeline_ci);

    ctx->dd->shader_free(tri_vs);
    ctx->dd->shader_free(tri_fs);

    return Error::Ok;
}

void GeometryFeature::destroy_resources() {}

void GeometryFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&geometry_pass);
};

}