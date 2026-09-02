#pragma once
#include <editor/docking/center_view/debug_tab.h>
#include <editor/docking/center_view/memory/memory_transients.h>
#include <cstdint>
#include <vector>

namespace lumen {
    
struct MemoryDebugTab : DebugTab
{
    uint64_t frame_counter = 0;
    uint64_t peak_bytes = 0;

    std::vector<float> detailed_frag;
    bool detailed_valid = false;

    MemoryTransients transients;
    
    const char* name() const override { return "Memory"; }
    void draw(EditorContext& ctx) override;
};

}