#pragma once
#include <core/rendering/features/feature.h>

namespace lumen {

struct ClusterCullFeature : Feature
{    
    RenderGraph::Pass clear_visible_pass;
    RenderGraph::Pass instance_cull_pass;
    RenderGraph::Pass cluster_expand_args_pass;
    RenderGraph::Pass cluster_expand_pass;
    RenderGraph::Pass cluster_cull_args_pass;
    RenderGraph::Pass cluster_cull_pass;
    
    drivers::DeviceDriverVulkan::Pipeline instance_cull_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_expand_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_expand_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_cull_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_cull_pipe;

    void _create_clear_visible_pass();
    void _create_instance_cull_pass();
    void _create_cluster_expand_args_pass();
    void _create_cluster_expand_pass();
    void _create_cluster_cull_args_pass();
    void _create_cluster_cull_pass();

    Error create_resources() override;
    Error create_pipelines() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}