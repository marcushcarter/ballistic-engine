#pragma once
#include <core/rendering/render_path/scene_render_path.h>
#include <core/rendering/features/editor/imgui_feature.h>
#include <core/rendering/features/editor/screenshot_feature.h>

namespace lumen {

struct EditorRenderPath : SceneRenderPath
{
    ImGuiFeature ui;
    ScreenshotFeature screenshot;    

    EditorRenderPath() {
        ui.sampled_image = "Out_Color";
        features.push_back(&ui);
        features.push_back(&screenshot);
    }
};

}