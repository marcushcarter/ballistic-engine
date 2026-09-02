#include <core/rendering/features/visibility_feature.h>

namespace lumen {

Error VisibilityFeature::create_resources()
{
    depth_pass.name = "Main_Depth";
    depth_pass.category = "DepthOnly";
    depth_pass.setup = [](RenderGraph::Builder& b) {
        (void)b;
        // drivers::DeviceDriverVulkan::ImageCreateInfo depth_ci{};
        // depth_ci.format = VK_FORMAT_D32_SFLOAT;
        // depth_ci.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        // depth_ci.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
        // b.create_image("G_Depth", depth_ci);
        // b.depth_attachment("G_Depth", VK_ATTACHMENT_LOAD_OP_CLEAR, [] { VkClearValue v{}; v.depthStencil = { 1.0f, 0 }; return v; }());
    };
    depth_pass.execute = [](RenderGraph::CommandList& cl) {
        (void)cl;
    };

    return Error::Ok;
};

void VisibilityFeature::destroy_resources()
{

}

void VisibilityFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&depth_pass);
};

}