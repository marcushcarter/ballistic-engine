#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/features/temp.h>
#include <core/rendering/features/visibility_feature.h>
#include <core/rendering/features/geometry_feature.h>
// #include <core/rendering/features/ambient_occlusion_feature.h>
// #include <core/rendering/features/deferred_lighting_feature.h>
// #include <core/rendering/features/subsurface_scattering_feature.h>

namespace lumen {

struct SceneRenderPath : RenderPath
{
    TempFeature temp;
    VisibilityFeature visibility;
    GeometryFeature geometry;
    // AmbientOcclusionFeature ao;
    // DeferredLightingFeature deferred_lighting;
    // SubsurfaceScatteringFeature subsurface_scattering;
    
    SceneRenderPath() {
        features.push_back(&temp);
        features.push_back(&visibility);
        features.push_back(&geometry);
        // features.push_back(&ao);
        // features.push_back(&deferred_lighting);
        // features.push_back(&subsurface_scattering);
    }
};
    
}