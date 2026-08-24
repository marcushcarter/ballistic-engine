#pragma once
#include <editor/editor_context.h>
#include <editor/panel/panel.h>
#include <core/base/error.h>
#include <editor/center_view/center_view.h>
#include <memory>
#include <vector>
#include <map>

namespace ballistic {

struct Editor
{    
    std::vector<std::unique_ptr<Panel>> panels;
    std::map<std::string, bool> panel_open;

    CenterView center_view;

    Error initialize();
    void shutdown();
    
    void _begin_dockspace();
    void on_update(EditorContext& ctx, float p_dt);
    void draw_menu();

    void take_screenshot(EditorContext& ctx);

    void apply_settings();
    void store_settings();
};

}