#pragma once
#include <core/rendering/features/feature.h>

namespace lumen {

struct ClusterCullFeature : Feature
{    
    RenderGraph::Pass clear_visible_pass;
    RenderGraph::Pass instance_cull_pass;
    RenderGraph::Pass cluster_refs_args_pass;
    RenderGraph::Pass cluster_refs_pass;
    RenderGraph::Pass cluster_cull_args_pass;
    RenderGraph::Pass cluster_cull_pass;
    RenderGraph::Pass raster_count_pass;
    RenderGraph::Pass raster_sum_pass;
    RenderGraph::Pass raster_emit_pass;
    RenderGraph::Pass raster_visibility_pass;
    RenderGraph::Pass hiz_build_pass;
    RenderGraph::Pass cluster_retest_args_pass;
    RenderGraph::Pass cluster_retest_pass;
    RenderGraph::Pass raster_count_pass_2;
    RenderGraph::Pass raster_sum_pass_2;
    RenderGraph::Pass raster_emit_pass_2;
    RenderGraph::Pass raster_visibility_pass_2;
    RenderGraph::Pass hiz_build_pass_2;
    RenderGraph::Pass material_resolve_pass;
    
    drivers::DeviceDriverVulkan::Pipeline instance_cull_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_refs_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_refs_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_cull_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_cull_pipe;
    drivers::DeviceDriverVulkan::Pipeline raster_count_pipe;
    drivers::DeviceDriverVulkan::Pipeline raster_sum_pipe;
    drivers::DeviceDriverVulkan::Pipeline raster_emit_pipe;
    drivers::DeviceDriverVulkan::Pipeline raster_visibility_pipe;
    drivers::DeviceDriverVulkan::Pipeline hiz_spd_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_retest_args_pipe;
    drivers::DeviceDriverVulkan::Pipeline cluster_retest_pipe;
    drivers::DeviceDriverVulkan::Pipeline material_resolve_pipe;

    void _create_clear_visible_pass();
    void _create_instance_cull_pass();
    void _create_cluster_refs_args_pass();
    void _create_cluster_refs_pass();
    void _create_cluster_cull_args_pass();
    void _create_cluster_cull_pass();
    void _create_raster_count_pass();
    void _create_raster_sum_pass();
    void _create_raster_emit_pass();
    void _create_raster_visibility_pass();
    void _create_hiz_build_pass();
    void _create_cluster_retest_args_pass();
    void _create_cluster_retest_pass();
    void _create_raster_count_2_pass();
    void _create_raster_sum_2_pass();
    void _create_raster_emit_2_pass();
    void _create_raster_visibility_2_pass();
    void _create_hiz_build_2_pass();
    void _create_material_resolve_pass();

    Error create_resources() override;
    Error create_pipelines() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}