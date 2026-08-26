#pragma once
#include <editor/asset_manager/asset_manager_grid.h>
#include <editor/asset_manager/asset_manager_toolbar.h>

#include <editor/asset_manager/asset_manager_list.h>

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

    AssetManagerList list;

    Error initialize();
    void shutdown();

    void _draw_folder_node(const std::filesystem::path& dir, std::filesystem::path& selected, int depth);
    void on_update(EditorContext& ctx);
};
    
}