#pragma once
#include <editor/editor_context.h>
#include <core/rendering/render_graph_profiler.h>

namespace lumen {

struct ProfilerResources
{
    void draw(EditorContext& ctx, const char* p_pass_name);
};

}