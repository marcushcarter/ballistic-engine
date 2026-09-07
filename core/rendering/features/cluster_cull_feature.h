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
    RenderGraph::Pass build_raster_args_pass;
    RenderGraph::Pass cluster_raster_pass;
    RenderGraph::Pass hiz_build_pass;
    RenderGraph::Pass cluster_retest_args_pass;
    
    drivers::DeviceDriverVulkan::Pipeline instance_cull_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_expand_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_expand_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_cull_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_cull_pipe;
    drivers::DeviceDriverVulkan::Pipeline build_raster_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_raster_pipe;
    drivers::DeviceDriverVulkan::Pipeline hiz_spd_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_retest_args_pipe;

    void _create_clear_visible_pass();
    void _create_instance_cull_pass();
    void _create_cluster_expand_args_pass();
    void _create_cluster_expand_pass();
    void _create_cluster_cull_args_pass();
    void _create_cluster_cull_pass();
    void _create_build_raster_args_pass();
    void _create_cluster_raster_pass();
    void _create_hiz_build_pass();
    void _create_cluster_retest_args_pass();

    Error create_resources() override;
    Error create_pipelines() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}