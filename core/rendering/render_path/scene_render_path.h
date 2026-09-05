#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/features/cluster_cull_feature.h>

namespace lumen {

struct SceneRenderPath : RenderPath
{
    ClusterCullFeature cluster_cull;
    
    SceneRenderPath() {
        features.push_back(&cluster_cull);
    }
};
    
}