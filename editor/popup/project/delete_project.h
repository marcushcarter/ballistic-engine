#pragma once
#include <editor/popup/popup.h>
#include <filesystem>
#include <string>

namespace lumen {

struct DeleteProjectPopup : Popup
{
    std::filesystem::path project_path;
    std::string project_name;
    
    const char* name() const override { return "Delete Project"; }
    void before_begin() override;
    void draw_contents(EditorContext& ctx) override;
    void draw_footer(EditorContext& ctx) override;
};

}