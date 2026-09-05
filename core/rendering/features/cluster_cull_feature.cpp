#include <core/rendering/features/cluster_cull_feature.h>
#include <core/rendering/resources/persistent_resources.h>
#include <core/io/embedded_resource.h>
#include <glm/glm.hpp>

namespace lumen {

using namespace glm;

Error ClusterCullFeature::create_resources()
{
    instance_cull_pass.name = "CC_InstanceCull";
    instance_cull_pass.category = "ClusterCull";
    instance_cull_pass.never_cull = true;
    instance_cull_pass.setup = [](RenderGraph::Builder& b) {
        drivers::DeviceDriverVulkan::BufferCreateInfo visible_ci{};
        visible_ci.size = (VkDeviceSize)MAX_INSTANCES * sizeof(uint32_t);
        visible_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        visible_ci.device_local = true;
        b.create_buffer("CC_VisibleInstances", visible_ci);
        
        drivers::DeviceDriverVulkan::BufferCreateInfo dispatch_ci{};
        dispatch_ci.size = sizeof(IndirectDispatch);
        dispatch_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        dispatch_ci.device_local = true;
        b.create_buffer("CC_ClusterCullDispatch", dispatch_ci);
        
        b.read_buffer("Camera", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("Geometry", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("MeshInstances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("Transforms", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("CC_VisibleInstances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.read_buffer("CC_ClusterCullDispatch", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    };
    instance_cull_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto camera = cl.graph->buffer("Camera");
        auto geometry = cl.graph->buffer("Geometry");
        auto mesh_inst = cl.graph->buffer("MeshInstances");
        auto tranforms = cl.graph->buffer("Transforms");
        auto visible = cl.graph->buffer("CC_VisibleInstances");
        auto dispatch = cl.graph->buffer("CC_ClusterCullDispatch");

        IndirectDispatch init{ 0, 1, 1 };
        vkCmdUpdateBuffer(cl.cmd, dispatch->buffer, 0, sizeof(init), &init);
        
        struct Push {
            VkDeviceAddress camera_addr;
            VkDeviceAddress geometry_addr;
            VkDeviceAddress instances_addr;
            VkDeviceAddress transforms_addr;
            VkDeviceAddress visible_addr;
            VkDeviceAddress dispatch_addr;
            uint32_t instance_count;
        } pc;
        
        pc.camera_addr = camera->device_address;
        pc.geometry_addr = geometry->device_address;
        pc.instances_addr = mesh_inst->device_address;
        pc.transforms_addr = tranforms->device_address;
        pc.visible_addr = visible->device_address;
        pc.dispatch_addr = dispatch->device_address;
        pc.instance_count = 1;

        // cl.dd->command_bind_pipeline(cl.cmd, instance_cull_pipe);
        // cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        // cl.dd->command_compute_dispatch(cl.cmd, (pc.instance_count + 63) / 64, 1, 1);
    };

    return Error::Ok;
};

Error ClusterCullFeature::create_pipelines()
{
    {
    EmbeddedResource::Blob comp_blob = EmbeddedResource::load(L"SHADERS_THIS_COMP");
    VkShaderModule cs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Compute, .glsl = (const char*)comp_blob.data, .glsl_size = comp_blob.size, .name = "CC_InstanceCullcs" });
    instance_cull_pipe = ctx->dd->compute_pipeline_create({cs, "CC_InstanceCull"});
    ctx->dd->shader_free(cs);
    }

    return Error::Ok;
}

void ClusterCullFeature::destroy_resources()
{

}

void ClusterCullFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&instance_cull_pass);
};

}

namespace lumen {

}