#pragma once
#include <editor/popup/popup.h>

namespace lumen {

struct ExportPopup : Popup
{
    const char* name() const override { return "Export"; }
    void draw_contents(EditorContext& ctx) override;
};

}