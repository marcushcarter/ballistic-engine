#pragma once
#include <editor/docking/panel.h>
#include <editor/docking/asset_browser/asset_browser_grid.h>
#include <editor/docking/asset_browser/asset_browser_toolbar.h>
#include <filesystem>

namespace ballistic {

struct AssetBrowserPanel : Panel
{
    std::filesystem::path selected_folder;
    char search_buf[256] = {};
    float left_width = 150.0f;

    AssetBrowserGrid grid;
    AssetBrowserToolbar toolbar;

    const char* name() const override { return "Asset Browser"; }
    void draw_contents(EditorContext& ctx) override;

    void _draw_divider_shadow(const ImVec2& region_p0, float region_h);
};

}