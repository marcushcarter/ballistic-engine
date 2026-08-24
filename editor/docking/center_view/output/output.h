#pragma once
#include <editor/docking/center_view/debug_tab.h>

namespace ballistic {
    
struct OutputDebugTab : DebugTab
{
    const char* name() const override { return "Ouput"; }
    void draw(EditorContext& ctx) override;
};

}