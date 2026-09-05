#pragma once
#include <core/rendering/features/feature.h>

namespace lumen {

struct ClusterCullFeature : Feature
{    
    RenderGraph::Pass instance_cull_pass;
    
    drivers::DeviceDriverVulkan::Pipeline instance_cull_pipe;

    Error create_resources() override;
    Error create_pipelines() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}