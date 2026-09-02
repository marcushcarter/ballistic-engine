#pragma once
#include <editor/docking/center_view/debugger.h>
#include <editor/docking/center_view/overlay_bar.h>
#include <editor/editor_context.h>
#include <imgui.h>
#include <cstdint>

namespace lumen {

struct CenterView
{
    Debugger debugger;
    OverlayBar left_overlay;
    OverlayBar right_overlay;
    
    uint64_t selected_name_id = 0;
    bool source_resolved = false;
    float split_ratio = 0.66f;

    void initialize();
    
    void _draw_scene(EditorContext& ctx);
    void draw(EditorContext& ctx);
};

}