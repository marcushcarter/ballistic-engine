#pragma once
#include <editor/popup/popup.h>

namespace lumen {

struct EditorSettingsPopup : Popup
{
    const char* name() const override { return "Editor Settings"; }
    void before_begin() override;
    void draw_contents(EditorContext& ctx) override;
};

}