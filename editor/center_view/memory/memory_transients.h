#pragma once
#include <editor/editor_context.h>
#include <core/rendering/render_graph_profiler.h>
#include <cstdint>

namespace ballistic {

struct MemoryTransients
{
    uint32_t max_rows = 100;

    void draw(EditorContext& ctx);
};

}