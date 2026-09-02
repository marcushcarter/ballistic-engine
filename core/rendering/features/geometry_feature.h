#pragma once
#include <core/rendering/features/feature.h>

namespace lumen {

struct GeometryFeature : Feature
{    
    RenderGraph::Pass geometry_pass;
    drivers::DeviceDriverVulkan::Pipeline triangle_pipeline;

    Error create_resources() override;
    Error create_pipelines() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}