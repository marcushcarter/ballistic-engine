#pragma once
#include <editor/editor_context.h>
#include <core/rendering/render_graph_profiler.h>

namespace ballistic {

struct ProfilerTimeline
{
    uint64_t sel_pass_key = 0;
    uint64_t sel_draw_key = 0;
    
    const RenderGraphProfiler::Timing* selected_pass = nullptr;
    const RenderGraphProfiler::Timing* selected_draw = nullptr;

    char sel_name[64] = {};
    bool follow = false;

    void draw(EditorContext& ctx);
};

}