#include <editor/editor_application.h>
#include <editor/popup/settings/editor_settings.h>
#include <editor/popup/settings/project_settings.h>
#include <editor/popup/project/new_project.h>
#include <editor/popup/project/delete_project.h>
#include <editor/popup/project/export.h>
#include <editor/popup/about/about_ballistic.h>
#include <drivers/toml/toml_helpers.h>
#include <core/io/embedded_resource.h>
#include <core/io/path.h>
#include <core/io/image_io.h>
#include <core/io/path.h>
#include <core/version.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <windows.h>
#include <shellapi.h>
#include <filesystem>

namespace ballistic {

Error EditorApplication::on_init()
{
    using enum Error;
    Error err;
    
    win32.window_set_custom_titlebar(true);
    win32.window_set_title("Ballistic Editor");

    err = resources.initialize(dd);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    err = win32.window_set_icon(EmbeddedResource::load_icon(L"BALLISTIC_ICON"));
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    ImVec4 titlebar = ImGui::GetStyle().Colors[ImGuiCol_MenuBarBg];
    err = win32.window_set_titlebar_color(RGB((BYTE)(titlebar.x * 255), (BYTE)(titlebar.y * 255), (BYTE)(titlebar.z * 255)));
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    
    ImGuiIO& io = ImGui::GetIO();
    {
        EmbeddedResource::Blob jb = EmbeddedResource::load(L"FONTS_JETBRAINS_MONO_REGULAR_TTF");
        ImFontConfig jb_cfg;
        jb_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF((void*)jb.data, (int)jb.size, 14.0f, &jb_cfg);

        EmbeddedResource::Blob fa = EmbeddedResource::load(L"FONTS_FA_SOLID_900_OTF");
        static const ImWchar fa_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        ImFontConfig fa_cfg;
        fa_cfg.MergeMode = true;
        fa_cfg.PixelSnapH = true;
        fa_cfg.FontDataOwnedByAtlas = false;
        io.Fonts->AddFontFromMemoryTTF((void*)fa.data, (int)fa.size, 14.0f, &fa_cfg, fa_ranges);
        io.Fonts->Build();
    }
    
    popups.register_popup(std::make_unique<EditorSettingsPopup>());
    popups.register_popup(std::make_unique<ProjectSettingsPopup>());
    popups.register_popup(std::make_unique<ExportPopup>());
    popups.register_popup(std::make_unique<NewProjectPopup>());
    popups.register_popup(std::make_unique<DeleteProjectPopup>());
    popups.register_popup(std::make_unique<AboutBallisticPopup>());

    err = project_manager.initialize();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);
    err = editor.initialize();
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    _load_state();
    settings.theme.apply();

    return Ok;
}

void EditorApplication::on_update(float p_dt)
{
    _draw_titlebar();

    EditorContext ctx = _make_context();
    
    popups.draw(ctx);
    if (project.loaded()) {
        
        imports.tick();
        for (const auto& c : imports.completed) {
            renderer.textures.unload(c.guid);
            renderer.textures.load(c.guid, c.content_bin);
        }
        imports.completed.clear();

        editor.on_update(ctx, p_dt);
    } else {
        project_manager.on_update(ctx);
    }
}

void EditorApplication::on_shutdown()
{
    resources.shutdown();
    
    _save_state();
    project_manager.save_recents();

    project_manager.shutdown();
    editor.shutdown();
}

Error EditorApplication::open_project(const std::filesystem::path& p_root)
{
    using enum Error;
    
    Error err = project_load(p_root);
    BALLISTIC_ERR_FAIL_COND_V(err != Ok, err);

    render_path_request(new EditorRenderPath());
    project_manager.add_recent(project.root, project.name);

    return Ok;
}

void EditorApplication::close_project()
{
    project_unload();
    render_path_request(new ProjectManagerRenderPath());
}

void EditorApplication::_load_state()
{
    std::ifstream in(Paths::roaming_data() / "editor_state.cfg", std::ios::binary);
    if (!in) return;

    toml::table parsed;
    try {
        parsed = toml::parse(in);
    } catch (const toml::parse_error&) {
        return;
    }
    const toml::table& tbl = parsed;

    auto lv = tbl.at_path("layout");
    if ((int)lv["version"].value_or((int64_t)-1) == Editor::VERSION) {
        editor.split_x = (float)lv["split_x"].value_or((double)editor.split_x);
        editor.split_y = (float)lv["split_y"].value_or((double)editor.split_y);
        editor.center_view.split_ratio        = (float)lv["center_split"].value_or((double)editor.center_view.split_ratio);
        editor.center_view.debugger.collapsed = lv["center_collapsed"].value_or(editor.center_view.debugger.collapsed);
        editor.center_view.debugger.active    = (int)lv["center_tab"].value_or((int64_t)editor.center_view.debugger.active);
        editor.right_top.active_name    = lv["active_top"].value_or(std::string{});
        editor.right_bottom.active_name = lv["active_bottom"].value_or(std::string{});

        if (const toml::table* pans = tbl.at_path("panels").as_table()) {
            for (auto& p : editor.panels) {
                auto pv = (*pans)[p->name()];
                p->open = pv["open"].value_or(p->open);
                p->zone = dock_zone_from_string(pv["zone"].value_or(std::string(to_string(p->zone))), p->zone);
            }
        }
    }

    settings.theme.preset = Theme::theme_preset_index(from_toml(tbl.at_path("theme.preset"), "Custom"));
    settings.theme.base = from_toml(tbl.at_path("theme.base"), settings.theme.base);
    settings.theme.accent = from_toml(tbl.at_path("theme.accent"), settings.theme.accent);
    settings.theme.text = from_toml(tbl.at_path("theme.text"), settings.theme.text);
    settings.theme.use_system_accent = tbl.at_path("theme.use_system_accent").value_or(settings.theme.use_system_accent);

    if (auto v = tbl.at_path("window.custom_titlebar").value<bool>()) win32.window_set_custom_titlebar(*v);

    renderer.graph.profiler.enabled = tbl.at_path("debugger.profiler.enabled").value_or(renderer.graph.profiler.enabled);

    settings.theme.apply();
}

void EditorApplication::_save_state()
{
    const char* preset_name = Theme::theme_preset_name(settings.theme.preset);

    toml::table theme;
    theme.insert_or_assign("preset", preset_name ? preset_name : "");
    theme.insert_or_assign("base", to_toml(settings.theme.base));
    theme.insert_or_assign("accent", to_toml(settings.theme.accent));
    theme.insert_or_assign("text", to_toml(settings.theme.text));
    theme.insert_or_assign("use_system_accent", settings.theme.use_system_accent);

    toml::table window;
    window.insert_or_assign("custom_titlebar", static_cast<bool>(win32.window.custom_titlebar));
    
    toml::table profiler;
    profiler.insert_or_assign("enabled", static_cast<bool>(renderer.graph.profiler.enabled));
    
    toml::table debugger;
    debugger.insert_or_assign("profiler", std::move(profiler));

    toml::table layout;
    layout.insert_or_assign("version", (int64_t)Editor::VERSION);
    layout.insert_or_assign("split_x", (double)editor.split_x);
    layout.insert_or_assign("split_y", (double)editor.split_y);
    layout.insert_or_assign("center_split", (double)editor.center_view.split_ratio);
    layout.insert_or_assign("center_collapsed", (bool)editor.center_view.debugger.collapsed);
    layout.insert_or_assign("center_tab", (int64_t)editor.center_view.debugger.active);
    layout.insert_or_assign("active_top", editor.right_top.active_name);
    layout.insert_or_assign("active_bottom", editor.right_bottom.active_name);

    toml::table panels;
    for (auto& p : editor.panels) {
        toml::table pt;
        pt.insert_or_assign("open", (bool)p->open);
        pt.insert_or_assign("zone", to_string(p->zone));
        panels.insert_or_assign(p->name(), std::move(pt));
    }

    toml::table root;
    root.insert_or_assign("theme", std::move(theme));
    root.insert_or_assign("window", std::move(window));
    root.insert_or_assign("debugger", std::move(debugger));
    
    root.insert_or_assign("layout", std::move(layout));
    root.insert_or_assign("panels", std::move(panels));

    std::ofstream out(Paths::roaming_data() / "editor_state.cfg", std::ios::binary);
    if (!out) return;
    out << root << '\n';
}

void EditorApplication::_draw_titlebar()
{
    const float TAB_H = 24.0f;
    const float BTN_W = 46.0f;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 7));

    const float MENU_H = ImGui::GetFrameHeight();
    const float H = MENU_H + TAB_H;
    const float LOGO = H;
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    if (!ImGui::BeginViewportSideBar("##BallisticTitlebar", ImGui::GetMainViewport(), ImGuiDir_Up, H, flags)) {
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }

    const ImVec2 origin = ImGui::GetWindowPos();
    const float width = ImGui::GetWindowWidth();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImDrawList* fg = ImGui::GetForegroundDrawList();

    win32.window_titlebar_reset((int)H);
    auto blocker = [&](ImVec2 mn, ImVec2 mx) {
        win32.window_titlebar_add_rect((long)(mn.x - origin.x), (long)(mn.y - origin.y), (long)(mx.x - origin.x), (long)(mx.y - origin.y));
    };

    if (ImGui::BeginMenuBar()) {
        
        ImGui::SetCursorPosX(LOGO + 6.0f);
        float menu_x0 = ImGui::GetCursorScreenPos().x;

        if (project.loaded()) _draw_shared_menu_items();
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("Online Documentation")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            if (ImGui::MenuItem("Forum")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            if (ImGui::MenuItem("Community")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            ImGui::Separator();
            if (ImGui::MenuItem("Copy System Info")) {
                const auto sys = win32.get_system_info();
                const auto gpu = dd.gpu_describe();
                char buf[1024];
                std::snprintf(buf, sizeof(buf),
                    "Ballistic v%d.%d.%d\n"
                    "OS:       %s (build %u)\n"
                    "Renderer: Vulkan %s (Deferred+)\n"
                    "GPU:      %s [%s]\n"
                    "          %s (%s)\n"
                    "          %.2f GiB VRAM\n"
                    "CPU:      %s\n"
                    "          %u cores / %u threads @ %u MHz\n"
                    "RAM:      %.2f GiB\n"
                    "Display:  %d monitor%s\n"
                    "Audio:    nah\n"
                    "Physics:  nah\n",
                    BALLISTIC_VERSION_MAJOR, BALLISTIC_VERSION_MINOR, BALLISTIC_VERSION_PATCH,
                    sys.os_name.c_str(), sys.os_build,
                    gpu.api_version.c_str(),
                    gpu.name.c_str(), gpu.type.c_str(),
                    gpu.driver_name.c_str(), gpu.driver_id.c_str(),
                    static_cast<double>(gpu.vram_bytes) / (1024.0 * 1024.0 * 1024.0),
                    sys.cpu_brand.c_str(),
                    sys.cpu_cores, sys.cpu_threads, sys.cpu_mhz,
                    static_cast<double>(sys.ram_total_bytes) / (1024.0 * 1024.0 * 1024.0),
                    sys.monitor_count, sys.monitor_count == 1 ? "" : "s"
                );
                ImGui::SetClipboardText(buf);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("About Ballistic")) popups.open("About Ballistic");
            if (ImGui::MenuItem("Support Development")) ShellExecuteA(nullptr, "open", "https://ballisticgames.ca", nullptr, nullptr, SW_SHOWNORMAL);
            ImGui::EndMenu();
        }

        float menu_x1 = ImGui::GetCursorScreenPos().x;
        if (menu_x1 > menu_x0) blocker(ImVec2(menu_x0, origin.y), ImVec2(menu_x1, origin.y + MENU_H));

        const std::string& title = project.name.empty() ? std::string("Ballistic Editor") : project.name;
        ImVec2 ts = ImGui::CalcTextSize(title.c_str());
        float btns_x = origin.x + width - BTN_W * 3.0f;
        float right_pad = win32.window.custom_titlebar ? (width - BTN_W * 3.0f) : width;
        float title_x = origin.x + right_pad - ts.x - 16.0f;
        dl->AddText(ImVec2(title_x, origin.y + (MENU_H - ts.y) * 0.5f), ImGui::GetColorU32(ImGuiCol_Text), title.c_str());
        
        if (win32.window.custom_titlebar) {
            float cluster_x = btns_x;
            float cluster_w = BTN_W * 3.0f;
            fg->AddRectFilled(ImVec2(cluster_x, origin.y), ImVec2(cluster_x + cluster_w, origin.y + MENU_H), ImGui::GetColorU32(ImGuiCol_MenuBarBg));

            RECT rmin, rmax, rclose;
            auto client_rect = [&](float x0) -> RECT {
                return RECT{ (long)(x0 - origin.x), (long)(origin.y - origin.y), (long)(x0 + BTN_W - origin.x), (long)(origin.y + MENU_H - origin.y) };
            };

            ImVec2 mouse = ImGui::GetIO().MousePos;
            auto ctrl = [&](float x0, int glyph, bool danger) {
                ImVec2 p(x0, origin.y);
                bool hovered = mouse.x >= x0 && mouse.x < x0 + BTN_W && mouse.y >= origin.y && mouse.y < origin.y + MENU_H;
                if (hovered) fg->AddRectFilled(p, ImVec2(p.x + BTN_W, p.y + MENU_H), danger ? IM_COL32(196, 43, 28, 255) : ImGui::GetColorU32(ImGuiCol_ButtonHovered));
                ImVec2 c(p.x + BTN_W * 0.5f, p.y + MENU_H * 0.5f);
                ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
                float s = 5.0f;
                switch (glyph) {
                    case 0: fg->AddLine(ImVec2(c.x-s,c.y), ImVec2(c.x+s,c.y), col, 1.0f); break;
                    case 1: fg->AddRect(ImVec2(c.x-s,c.y-s), ImVec2(c.x+s,c.y+s), col, 0,0,1.0f); break;
                    case 2:
                        fg->AddRect(ImVec2(c.x-s+2,c.y-s-2), ImVec2(c.x+s+2,c.y+s-2), col, 0,0,1.0f);
                        fg->AddRectFilled(ImVec2(c.x-s-2,c.y-s+2), ImVec2(c.x+s-2,c.y+s+2), ImGui::GetColorU32(ImGuiCol_MenuBarBg));
                        fg->AddRect(ImVec2(c.x-s-2,c.y-s+2), ImVec2(c.x+s-2,c.y+s+2), col, 0,0,1.0f);
                        break;
                    case 3:
                        fg->AddLine(ImVec2(c.x-s,c.y-s), ImVec2(c.x+s,c.y+s), col, 1.2f);
                        fg->AddLine(ImVec2(c.x-s,c.y+s), ImVec2(c.x+s,c.y-s), col, 1.2f);
                        break;
                }
            };

            float x_min = btns_x;
            float x_max = btns_x + BTN_W;
            float x_close = btns_x + BTN_W * 2.0f;

            ctrl(x_min, 0, false);
            ctrl(x_max, win32.window_is_maximized() ? 2 : 1, false);
            ctrl(x_close, 3, true);

            rmin = client_rect(x_min);
            rmax = client_rect(x_max);
            rclose = client_rect(x_close);
            win32.window_titlebar_set_controls(rmin, rmax, rclose);
        }

        ImGui::EndMenuBar();
    }

    const ImU32 bar_bg = ImGui::GetColorU32(ImGuiCol_MenuBarBg);
    dl->AddRectFilled(ImVec2(origin.x, origin.y + MENU_H), ImVec2(origin.x + width, origin.y + H), bar_bg); 

    ImGui::SetCursorScreenPos(ImVec2(origin.x + LOGO + 6.0f, origin.y + MENU_H));
    const float TAB_PAD_Y = (TAB_H - ImGui::GetFontSize()) * 0.5f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14, TAB_PAD_Y));
    ImGui::PushStyleVar(ImGuiStyleVar_TabRounding, 4.0f);
    if (ImGui::BeginTabBar("##TitlebarTabs", ImGuiTabBarFlags_AutoSelectNewTabs)) {
        for (int i = 0; i < (int)titlebar_tabs.size(); ++i) {
            ImGui::PushID(i);
            bool selected = ImGui::BeginTabItem(titlebar_tabs[i].c_str());
            blocker(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            if (selected) { titlebar_active_tab = i; ImGui::EndTabItem(); }
            ImGui::PopID();
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleVar(2);

    {
        const char* cog = ICON_FA_GEAR;
        ImVec2 cog_sz = ImGui::CalcTextSize(cog);
        float pad = 12.0f;
        float box = TAB_H;
        ImVec2 cog_pos(origin.x + width - pad - box, origin.y + MENU_H);

        ImGui::SetCursorScreenPos(cog_pos);
        ImGui::InvisibleButton("##settings_cog", ImVec2(box, box));
        blocker(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

        bool hovered = ImGui::IsItemHovered();
        if (ImGui::IsItemClicked()) popups.open("Editor Settings");

        ImU32 col = hovered ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
        dl->AddText(ImVec2(cog_pos.x + (box - cog_sz.x) * 0.5f, cog_pos.y + (box - cog_sz.y) * 0.5f), col, cog);
    }

    {
        VkDescriptorSet logo_set = imgui.texture_cache.get(resources.icon_image.image_view);
        dl->PushClipRect(origin, ImVec2(origin.x + width, origin.y + H), false);
        float m = 6.0f;
        ImVec2 mn(origin.x + m, origin.y + m);
        ImVec2 mx(origin.x + LOGO - m, origin.y + H - m);
        if (logo_set) dl->AddImage(logo_set, mn, mx);
        else dl->AddRectFilled(mn, mx, ImGui::GetColorU32(ImGuiCol_Text), 4.0f);
        dl->PopClipRect();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorApplication::_draw_shared_menu_items()
{
    if (ImGui::BeginMenu("File")) {
        // search bar
        ImGui::Separator();

        // if (ImGui::MenuItem("New")) {}
        // if (ImGui::MenuItem("Open")) {}
        // if (ImGui::MenuItem("Export Scene")) {}

        ImGui::Separator();
        
        // if (ImGui::MenuItem("Open Asset")) {}

        ImGui::Separator();

        // if (ImGui::MenuItem("Save Current Scene")) {}
        // if (ImGui::MenuItem("Save Current Scene As")) {}
        if (ImGui::MenuItem("Save All")) project.save();
        // if (ImGui::MenuItem("Choose Files to Save")) {}

        ImGui::Separator();
        
        // if (ImGui::MenuItem("Import Into Scene")) {}
        // if (ImGui::MenuItem("Export All")) {}
        
        // if (ImGui::MenuItem("Take Screenshot", "Ctrl+F12")) {   
        //     EditorRenderPath* path = static_cast<EditorRenderPath*>(render_path);
        //     path->screenshot.requested = true;
        // }

        ImGui::Separator();
        
        // if (ImGui::MenuItem("New")) {}
        // if (ImGui::MenuItem("Open")) {}
        // if (ImGui::MenuItem("Zip Project")) {}
        if (ImGui::MenuItem("Open Current Project Directory")) Paths::reveal_in_explorer(project.root);
        // if (ImGui::MenuItem("Recent Projects")) {}

        ImGui::Separator();        

        if (ImGui::MenuItem("Exit")) close_project();
        if (ImGui::MenuItem("Quit", "Alt+F4")) win32.window_request_close();
        
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {        
        // search bar
        ImGui::Separator();
        
        // if (ImGui::MenuItem("Undo")) {}
        // if (ImGui::MenuItem("Redo")) {}
        // if (ImGui::MenuItem("Undo History")) {}
        
        ImGui::Separator();
        
        // ImGui::BeginDisabled(true);
        // if (ImGui::MenuItem("Cut")) {}
        // if (ImGui::MenuItem("Copy")) {}
        // if (ImGui::MenuItem("Paste")) {}
        // if (ImGui::MenuItem("Duplicate")) {}
        // if (ImGui::MenuItem("Delete")) {}
        // ImGui::EndDisabled();
        
        ImGui::Separator();

        if (ImGui::MenuItem("Editor Settings")) popups.open("Editor Settings");
        if (ImGui::MenuItem("Project Settings")) popups.open("Project Settings");
        // if (ImGui::MenuItem("Keyboard Shortcuts")) {}
        // if (ImGui::MenuItem("Plugins")) {}
        
        if (ImGui::MenuItem("Open Editor Data Folder")) Paths::reveal_in_explorer(Paths::roaming_data());
        
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window")) {
        // search bar
        ImGui::Separator();

        editor.draw_menu();
        
        ImGui::Separator();
        
        // if (ImGui::MenuItem("Device Output")) {}
        // if (ImGui::MenuItem("Message")) {}
        // if (ImGui::MenuItem("Output Log")) {}
        
        ImGui::Separator();

        // if (ImGui::MenuItem("Enable Fullscreen", "Alt+F11")) {}
        
        ImGui::Separator();
        
        ImGui::EndMenu();
    }
    
    if (ImGui::BeginMenu("Tools")) {
        // search bar
        ImGui::Separator();
        
        // if (ImGui::MenuItem("Render Resource Viewer")) {}
        
        ImGui::Separator();

        // if (ImGui::BeginMenu("Debug")) {

        //     ImGui::EndMenu();
        // }
        
        // if (ImGui::BeginMenu("Profiler")) {

        //     ImGui::EndMenu();
        // }
        
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Export")) {
        // search bar
        ImGui::Separator();
        
        // if (ImGui::MenuItem("Play In Editor Current Scene")) {}
        if (ImGui::MenuItem("Export")) popups.open("Export");
        
        ImGui::EndMenu();
    }
}

EditorContext EditorApplication::_make_context()
{
    EditorContext ctx{};
    ctx.win32 = &win32;
    ctx.imgui = &imgui;
    ctx.renderer = &renderer;
    ctx.render_path = static_cast<EditorRenderPath*>(render_path);
    ctx.project = &project;
    ctx.tasks = &tasks;
    
    ctx.settings = &settings;
    ctx.resources = &resources;
    ctx.imports = &imports;
    
    ctx.project_manager = &project_manager;
    ctx.editor = &editor;
    ctx.popups = &popups;

    ctx.open_project_callback = [this](const auto& path){this->open_project(path);};
    ctx.close_project_callback = [this](){this->close_project();};
    
    return ctx;
}

}