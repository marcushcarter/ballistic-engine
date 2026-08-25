#pragma once
#include <editor/asset_manager/asset_manager_grid.h>
#include <editor/asset_manager/asset_manager_toolbar.h>
#include <editor/editor_context.h>
#include <core/base/error.h>
#include <imgui.h>
#include <filesystem>

namespace ballistic {

struct AssetManager
{
    std::filesystem::path selected_folder;
    char search_buf[256] = {};
    float split_x = 0.18f;

    AssetBrowserGrid grid;
    AssetBrowserToolbar toolbar;

    Error initialize();
    void shutdown();

    void on_update(EditorContext& ctx);

    void _draw_divider_shadow(const ImVec2& region_p0, float region_h, float left_w);
};
    
}