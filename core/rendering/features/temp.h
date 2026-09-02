#pragma once
#include <core/rendering/features/feature.h>

namespace lumen {

struct TempFeature : Feature
{
    RenderGraph::Pass pass;

    Error create_resources() override
    {
        pass.name = "Placeholder";
        pass.category = "Placeholder";
        pass.setup = [](RenderGraph::Builder& b) {
            drivers::DeviceDriverVulkan::ImageCreateInfo image_ci{};
            image_ci.name = "Out_Color";
            image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
            image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            b.create_image("Out_Color", image_ci);
            // b.color_attachment("Out_Color", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.1f, 0.2f, 0.8f, 1.0f } });

            // drivers::DeviceDriverVulkan::ImageCreateInfo depth_ci{};
            // image_ci.name = "Depth";
            // depth_ci.format = VK_FORMAT_D32_SFLOAT;
            // depth_ci.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            // depth_ci.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
            // b.create_image("Depth", depth_ci);
            // b.depth_attachment("Depth", VK_ATTACHMENT_LOAD_OP_CLEAR, [] { VkClearValue v{}; v.depthStencil = { 1.0f, 0 }; return v; }());

            // drivers::DeviceDriverVulkan::ImageCreateInfo image_ci{};
            // image_ci.name = "Color";
            // image_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
            // image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            // b.create_image("Color", image_ci);
            // b.color_attachment("Color", VK_ATTACHMENT_LOAD_OP_CLEAR, { { 0.0f, 1.0f, 0.0f, 1.0f } });

            // drivers::DeviceDriverVulkan::BufferCreateInfo buffer_ci{};
            // buffer_ci.name = "Buffer";
            // buffer_ci.size = 3 * 1024 * 1024;
            // buffer_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            // buffer_ci.memory = drivers::DeviceDriverVulkan::BufferCreateInfo::Memory::DeviceLocal;
            // b.create_buffer("Buffer", buffer_ci);
            // b.write_buffer("Buffer", VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        };

        pass.execute = [](RenderGraph::CommandList& cl) {
            (void)cl;
        };

        return Error::Ok;
    };

    void build(RenderGraph& g) override {
        if (!enabled) return;
        g.add(&pass);
    };
};

}