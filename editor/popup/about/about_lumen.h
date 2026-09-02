#pragma once
#include <editor/popup/popup.h>

namespace lumen {

struct AboutLumenPopup : Popup
{
    const char* name() const override { return "About Lumen"; }
    void before_begin() override;
    void draw_contents(EditorContext& ctx) override;
};

}