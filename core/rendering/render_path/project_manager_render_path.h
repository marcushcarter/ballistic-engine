#pragma once
#include <core/rendering/render_path/render_path.h>
#include <core/rendering/features/editor/imgui_feature.h>
#include <core/rendering/features/editor/screenshot_feature.h>

namespace lumen {

struct ProjectManagerRenderPath : RenderPath
{
    ImGuiFeature ui;
    ScreenshotFeature screenshot;  

    ProjectManagerRenderPath() {
        features.push_back(&ui);
        features.push_back(&screenshot);
    }
};

}