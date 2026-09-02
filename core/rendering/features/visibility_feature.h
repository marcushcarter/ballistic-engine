#pragma once
#include <core/rendering/features/feature.h>

namespace lumen {

struct VisibilityFeature : Feature
{    
    RenderGraph::Pass depth_pass;

    Error create_resources() override;
    void destroy_resources() override;
    void build(RenderGraph& g) override;
};

}