#pragma once
#include <editor/center_view/debugger.h>
#include <editor/editor_context.h>
#include <imgui.h>
#include <cstdint>

namespace ballistic {

struct CenterView
{
    Debugger debugger;
    
    uint64_t selected_name_id = 0;
    float split_ratio = 0.5f; 

    void initialize();
    
    void _draw_scene(EditorContext& ctx);
    void draw(EditorContext& ctx);
};

}