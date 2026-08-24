#pragma once
#include <editor/docking/center_view/debug_tab.h>
#include <editor/docking/center_view/profiler/profiler_timeline.h>
#include <editor/docking/center_view/profiler/profiler_distribution.h>
#include <editor/docking/center_view/profiler/profiler_resources.h>
#include <cstdint>

namespace ballistic {
    
struct ProfilerDebugTab : DebugTab
{
    ProfilerTimeline timeline;
    ProfilerDistribution distribution;
    ProfilerResources resources;

    const char* name() const override { return "Profiler"; }
    void draw(EditorContext& ctx) override;
};

}