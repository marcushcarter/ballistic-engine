#pragma once
#include <editor/popup/popup.h>
#include <filesystem>

namespace lumen {

struct NewProjectPopup : Popup
{
    char name_buf[128] = {};
    char location_buf[512] = {};
    bool create_folder = true;
    bool edit_now = true;

    bool can_create = false;    
    std::filesystem::path final_path;
    
    const char* name() const override { return "New Project"; }
    void before_begin() override;
    void on_open(EditorContext& ctx) override;
    void draw_contents(EditorContext& ctx) override;
    void draw_footer(EditorContext& ctx) override;
};

}