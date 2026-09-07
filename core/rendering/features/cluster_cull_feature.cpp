#include <core/rendering/features/cluster_cull_feature.h>
#include <core/rendering/frame_data.h>
#include <core/io/embedded_resource.h>
#include <glm/glm.hpp>

namespace lumen {

using namespace glm;

void ClusterCullFeature::_create_clear_visible_pass()
{
    clear_visible_pass.name = "ClearVisibleInstances";
    clear_visible_pass.category = "ClusterCull";
    clear_visible_pass.setup = [this](RenderGraph::Builder& b) {
        drivers::DeviceDriverVulkan::BufferCreateInfo visible_ci{};
        visible_ci.size = (VkDeviceSize)(ctx->frame->instance_count + 1) * sizeof(uint32_t);
        visible_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        visible_ci.device_local = true;
        b.create_buffer("VisibleInstances", visible_ci);
        b.write_buffer("VisibleInstances", VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);
    };
    clear_visible_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto visible = cl.graph->buffer("VisibleInstances");
        cl.fill_buffer("Clear visible instances", *visible, 0, 0, sizeof(uint32_t));
    };
}

void ClusterCullFeature::_create_instance_cull_pass()
{
    instance_cull_pass.name = "InstanceCull";
    instance_cull_pass.category = "ClusterCull";
    instance_cull_pass.setup = [](RenderGraph::Builder& b) {        
        b.read_buffer("Camera", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("Geometry", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("Instances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("Transforms", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.write_buffer("VisibleInstances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    };
    instance_cull_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto camera = cl.graph->buffer("Camera");
        auto geometry = cl.graph->buffer("Geometry");
        auto inst = cl.graph->buffer("Instances");
        auto tranforms = cl.graph->buffer("Transforms");
        auto visible = cl.graph->buffer("VisibleInstances");

        struct Push {
            VkDeviceAddress camera_addr;
            VkDeviceAddress geometry_addr;
            VkDeviceAddress instances_addr;
            VkDeviceAddress transforms_addr;
            VkDeviceAddress visible_addr;
            uint32_t instance_count;
        } pc;
        pc.camera_addr = camera->device_address;
        pc.geometry_addr = geometry->device_address;
        pc.instances_addr = inst->device_address;
        pc.transforms_addr = tranforms->device_address;
        pc.visible_addr = visible->device_address;
        pc.instance_count = ctx->frame->instance_count;

        cl.dd->command_bind_pipeline(cl.cmd, instance_cull_pipe);
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        cl.dispatch("Instance cull", (ctx->frame->instance_count + 63) / 64);
    };    
}

void ClusterCullFeature::_create_cluster_expand_args_pass()
{
    cluster_expand_args_pass.name = "ClusterExpandArgs";
    cluster_expand_args_pass.category = "ClusterCull";
    cluster_expand_args_pass.never_cull = true;
    cluster_expand_args_pass.setup = [this](RenderGraph::Builder& b) {
        drivers::DeviceDriverVulkan::BufferCreateInfo args_ci{};
        args_ci.size = sizeof(IndirectDispatch);
        args_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        args_ci.device_local = true;
        b.create_buffer("ClusterExpandArgs", args_ci);

        drivers::DeviceDriverVulkan::BufferCreateInfo refs_ci{};
        refs_ci.size = (VkDeviceSize)(ctx->frame->cluster_ref_capacity + 1) * sizeof(uint64_t);
        refs_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        refs_ci.device_local = true;
        b.create_buffer("ClusterRefs", refs_ci);
        
        b.read_buffer("VisibleInstances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.write_buffer("ClusterExpandArgs", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.write_buffer("ClusterRefs", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    };
    cluster_expand_args_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto vis_inst = cl.graph->buffer("VisibleInstances");
        auto expand_args = cl.graph->buffer("ClusterExpandArgs");
        auto cluster_refs = cl.graph->buffer("ClusterRefs");
        
        struct Push {
            VkDeviceAddress visible_inst_addr;
            VkDeviceAddress expand_addr;
            VkDeviceAddress cluster_refs_addr;
        } pc;
        pc.visible_inst_addr = vis_inst->device_address;
        pc.expand_addr = expand_args->device_address;
        pc.cluster_refs_addr = cluster_refs->device_address;

        cl.dd->command_bind_pipeline(cl.cmd, cluster_expand_args_pipe);
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        cl.dispatch("Cluster expand args", 1);
    };
}

void ClusterCullFeature::_create_cluster_expand_pass()
{
    cluster_expand_pass.name = "ClusterExpand";
    cluster_expand_pass.category = "ClusterCull";
    cluster_expand_pass.never_cull = true;
    cluster_expand_pass.setup = [this](RenderGraph::Builder& b) {
        b.read_buffer("Geometry", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("Instances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("VisibleInstances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("ClusterExpandArgs", VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT);
        b.write_buffer("ClusterRefs", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    };
    cluster_expand_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto geometry = cl.graph->buffer("Geometry");
        auto inst = cl.graph->buffer("Instances");
        auto visible = cl.graph->buffer("VisibleInstances");
        auto expand_args = cl.graph->buffer("ClusterExpandArgs");
        auto cluster_refs = cl.graph->buffer("ClusterRefs");
        
        struct Push {
            VkDeviceAddress geometry_addr;
            VkDeviceAddress instances_addr;
            VkDeviceAddress visible_addr;
            VkDeviceAddress cluster_refs_addr;
        } pc;
        pc.geometry_addr = geometry->device_address;
        pc.instances_addr = inst->device_address;
        pc.visible_addr = visible->device_address;
        pc.cluster_refs_addr = cluster_refs->device_address;

        cl.dd->command_bind_pipeline(cl.cmd, cluster_expand_pipe);
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        cl.dispatch_indirect("Cluster expand", *expand_args);
    };
}

void ClusterCullFeature::_create_cluster_cull_args_pass()
{
    cluster_cull_args_pass.name = "ClusterCullArgs";
    cluster_cull_args_pass.category = "ClusterCull";
    cluster_cull_args_pass.never_cull = true;
    cluster_cull_args_pass.setup = [this](RenderGraph::Builder& b) {
        drivers::DeviceDriverVulkan::BufferCreateInfo args_ci{};
        args_ci.size = sizeof(IndirectDispatch);
        args_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        args_ci.device_local = true;
        b.create_buffer("ClusterCullArgs", args_ci);
        
        drivers::DeviceDriverVulkan::BufferCreateInfo visible_ci{};
        visible_ci.size = (VkDeviceSize)(ctx->frame->cluster_ref_capacity + 1) * sizeof(uint64_t);
        visible_ci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        visible_ci.device_local = true;
        b.create_buffer("VisibleClusters", visible_ci);
        
        b.read_buffer("ClusterRefs", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.write_buffer("ClusterCullArgs", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
        b.write_buffer("VisibleClusters", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_TRANSFER_WRITE_BIT);
    };
    cluster_cull_args_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto cluster_refs = cl.graph->buffer("ClusterRefs");
        auto cull_args = cl.graph->buffer("ClusterCullArgs");
        auto vis_clus = cl.graph->buffer("VisibleClusters");
        
        struct Push {
            VkDeviceAddress cluster_refs_addr;
            VkDeviceAddress cull_addr;
            VkDeviceAddress vis_clus_addr;
        } pc;
        pc.cluster_refs_addr = cluster_refs->device_address;
        pc.cull_addr = cull_args->device_address;
        pc.vis_clus_addr = vis_clus->device_address;

        cl.dd->command_bind_pipeline(cl.cmd, cluster_cull_args_pipe);
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        cl.dispatch("Cluster cull args", 1);
    };
}

void ClusterCullFeature::_create_cluster_cull_pass()
{
    cluster_cull_pass.name = "ClusterCull";
    cluster_cull_pass.category = "ClusterCull";
    cluster_cull_pass.never_cull = true;
    cluster_cull_pass.setup = [this](RenderGraph::Builder& b) {
        b.read_buffer("Camera", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("Geometry", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_UNIFORM_READ_BIT);
        b.read_buffer("Instances", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("Transforms", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("ClusterRefs", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT);
        b.read_buffer("ClusterCullArgs", VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT, VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT);
        b.write_buffer("VisibleClusters", VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT, VK_ACCESS_2_SHADER_STORAGE_READ_BIT | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT);
    };
    cluster_cull_pass.execute = [this](RenderGraph::CommandList& cl) {
        auto camera = cl.graph->buffer("Camera");
        auto geometry = cl.graph->buffer("Geometry");
        auto inst = cl.graph->buffer("Instances");
        auto transforms = cl.graph->buffer("Transforms");
        auto cluster_refs = cl.graph->buffer("ClusterRefs");
        auto cull_args = cl.graph->buffer("ClusterCullArgs");
        auto vis_clus = cl.graph->buffer("VisibleClusters");
        
        struct Push {
            VkDeviceAddress camera_addr;
            VkDeviceAddress geometry_addr;
            VkDeviceAddress instances_addr;
            VkDeviceAddress transforms_addr;
            VkDeviceAddress cluster_refs_addr;
            VkDeviceAddress visible_clusters_addr;
        } pc;
        pc.camera_addr = camera->device_address;
        pc.geometry_addr = geometry->device_address;
        pc.instances_addr = inst->device_address;
        pc.transforms_addr = transforms->device_address;
        pc.cluster_refs_addr = cluster_refs->device_address;
        pc.visible_clusters_addr = vis_clus->device_address;

        cl.dd->command_bind_pipeline(cl.cmd, cluster_cull_pipe);
        cl.dd->command_bind_push_constants(cl.cmd, sizeof(pc), &pc);
        cl.dispatch_indirect("Cluster cull 1", *cull_args);
    };
}

Error ClusterCullFeature::create_resources()
{
    _create_clear_visible_pass();
    _create_instance_cull_pass();
    _create_cluster_expand_args_pass();
    _create_cluster_expand_pass();
    _create_cluster_cull_args_pass();
    _create_cluster_cull_pass();

    return Error::Ok;
};

Error ClusterCullFeature::create_pipelines()
{
    using enum Error;

    {
    EmbeddedResource::Blob comp_blob = EmbeddedResource::load(L"SHADERS_CLUSTER_CULL_INSTANCE_CULL_COMP");
    VkShaderModule cs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Compute, .glsl = (const char*)comp_blob.data, .glsl_size = comp_blob.size, .name = "cluster_cull/instance_cull.comp" });
    instance_cull_pipe = ctx->dd->compute_pipeline_create({cs, "cluster_cull/instance_cull"});
    ctx->dd->shader_free(cs);
    }
    
    {
    EmbeddedResource::Blob comp_blob = EmbeddedResource::load(L"SHADERS_CLUSTER_CULL_CLUSTER_EXPAND_ARGS_COMP");
    VkShaderModule cs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Compute, .glsl = (const char*)comp_blob.data, .glsl_size = comp_blob.size, .name = "cluster_cull/cluster_expand_args.comp" });
    cluster_expand_args_pipe = ctx->dd->compute_pipeline_create({cs, "cluster_cull/cluster_expand_args"});
    ctx->dd->shader_free(cs);
    }
    
    {
    EmbeddedResource::Blob comp_blob = EmbeddedResource::load(L"SHADERS_CLUSTER_CULL_CLUSTER_EXPAND_COMP");
    VkShaderModule cs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Compute, .glsl = (const char*)comp_blob.data, .glsl_size = comp_blob.size, .name = "cluster_cull/cluster_expand.comp" });
    cluster_expand_pipe = ctx->dd->compute_pipeline_create({cs, "cluster_cull/cluster_expand"});
    ctx->dd->shader_free(cs);
    }
    
    {
    EmbeddedResource::Blob comp_blob = EmbeddedResource::load(L"SHADERS_CLUSTER_CULL_CLUSTER_CULL_ARGS_COMP");
    VkShaderModule cs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Compute, .glsl = (const char*)comp_blob.data, .glsl_size = comp_blob.size, .name = "cluster_cull/cluster_cull_args.comp" });
    cluster_cull_args_pipe = ctx->dd->compute_pipeline_create({cs, "cluster_cull/cluster_cull_args"});
    ctx->dd->shader_free(cs);
    }
    
    {
    EmbeddedResource::Blob comp_blob = EmbeddedResource::load(L"SHADERS_CLUSTER_CULL_CLUSTER_CULL_COMP");
    VkShaderModule cs = ctx->dd->shader_create({ .stage = drivers::DeviceDriverVulkan::ShaderStage::Compute, .glsl = (const char*)comp_blob.data, .glsl_size = comp_blob.size, .name = "cluster_cull/cluster_cull.comp" });
    cluster_cull_pipe = ctx->dd->compute_pipeline_create({cs, "cluster_cull/cluster_cull"});
    ctx->dd->shader_free(cs);
    }

    return Ok;
}

void ClusterCullFeature::destroy_resources()
{
    ctx->dd->pipeline_free(instance_cull_pipe);
    ctx->dd->pipeline_free(cluster_expand_args_pipe);
    ctx->dd->pipeline_free(cluster_expand_pipe);
    ctx->dd->pipeline_free(cluster_cull_args_pipe);
    ctx->dd->pipeline_free(cluster_cull_pipe);
}

void ClusterCullFeature::build(RenderGraph& g)
{
    if (!enabled) return;
    g.add(&clear_visible_pass);
    g.add(&instance_cull_pass);
    g.add(&cluster_expand_args_pass);
    g.add(&cluster_expand_pass);
    g.add(&cluster_cull_args_pass);
    g.add(&cluster_cull_pass);
};

}