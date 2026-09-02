#pragma once
#include <editor/editor_context.h>
#include <core/rendering/render_graph_profiler.h>
#include <imgui.h>
#include <vector>

namespace lumen {

struct ProfilerDistribution
{
    struct ScrollBuf {
        int max = 256;
        ImVector<float> data;
        void add(float v) {
            data.push_back(v);
            if (data.Size > max) data.erase(data.Data);
        }
        void clear() { data.shrink(0); }
    };
    
    uint64_t plot_key = 0;
    ScrollBuf plot_hist;
    std::vector<float> sort_scratch;

    void draw(EditorContext& ctx, const RenderGraphProfiler::Timing* selected, bool frozen);
};

}