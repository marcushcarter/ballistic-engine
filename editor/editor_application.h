
#pragma once
#include <core/application/application.h>
#include <editor/docking/editor.h>
#include <editor/editor_mode/project_manager.h>
#include <editor/editor_settings.h>
#include <editor/editor_resources.h>
#include <editor/assets/asset_import_tracker.h>
#include <editor/popup/popup_manager.h>
#include <core/rendering/render_path/editor_render_path.h>
#include <core/rendering/render_path/project_manager_render_path.h>
#include <vector>

namespace ballistic {

struct EditorApplication : Application
{
    ProjectManager project_manager;
    Editor editor;
    PopupManager popups;

    EditorSettings settings;
    EditorResources resources;
    AssetImportTracker imports;

    std::vector<std::string> titlebar_tabs { "Level", "Text Editor", "Particles" };
    int titlebar_active_tab = 0;

    Error on_init() override;
    void on_update(float p_dt) override;
    void on_shutdown() override;

    Error open_project(const std::filesystem::path& p_root);
    void close_project();

    void _load_state();
    void _save_state();

    void _draw_titlebar();
    void _draw_shared_menu_items();

    EditorContext _make_context();

    bool wants_docking() const override { return true; }
    bool wants_custom_titlebar() const override { return false; }
    
    RenderPath* create_render_path() { return new EditorRenderPath(); }
};

}