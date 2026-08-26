#include <editor/docking/editor.h>
#include <drivers/imgui/imgui_helpers.h>
#include <core/rendering/render_path/editor_render_path.h>
#include <IconsFontAwesome6.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome6.h>

namespace ballistic {

Error Editor::initialize()
{
    using enum Error;
    center_view.initialize();
    right_top.zone = DockZone::RightTop;
    right_bottom.zone = DockZone::RightBottom;
    return Ok;
}

void Editor::shutdown()
{
    right_top.panels.clear();
    right_bottom.panels.clear();
    panels.clear();
}

void Editor::_draw_toolbar(EditorContext&)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 4));

    ImGui::Button(ICON_FA_HARD_DRIVE);

    // if (ImGui::Button(ICON_FA_PLAY)) {}
    // ImGui::SameLine();
    // if (ImGui::Button(ICON_FA_STOP)) {}
    // ImGui::SameLine();
    // ImGui::TextDisabled("Ballistic");
    // ImGui::SameLine();

    // ImGui::Text("panels=%d, rtop=%d, rbot=%d",
    //             (int)panels.size(),
    //             (int)right_top.panels.size(),
    //             (int)right_bottom.panels.size());

    ImGui::PopStyleVar();
}

void Editor::on_update(EditorContext& ctx, float)
{
    right_top.panels.clear();
    right_bottom.panels.clear();
    for (auto& p : panels) {
        switch (p->zone) {
            case DockZone::RightTop: right_top.panels.push_back(p.get());    break;
            case DockZone::RightBottom: right_bottom.panels.push_back(p.get()); break;
            case DockZone::Left: break;
        }
    }

    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##EditorHost", nullptr,
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus
    );
    ImGui::PopStyleVar(3);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    ImGui::BeginChild("##topbar", ImVec2(0, bar_h), true, ImGuiWindowFlags_NoScrollbar);
    _draw_toolbar(ctx);
    ImGui::EndChild();
    ImGui::PopStyleVar();

    const float thick = 6.0f;
    ImVec2 avail = ImGui::GetContentRegionAvail();

    float body_w = avail.x - thick;
    if (body_w < 1.0f) body_w = 1.0f;
    const float min_side = 120.0f;
    split_x = ImClamp(split_x, min_side / body_w, 1.0f - min_side / body_w);
    float left_w  = ImFloor(body_w * split_x);
    float right_w = body_w - left_w;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("##left", ImVec2(left_w, avail.y), false, ImGuiWindowFlags_NoScrollbar);
    center_view.draw(ctx);
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::SameLine(0, 0);
    SplitterState sx = imgui_splitter("##split_lr", SplitAxis::X, ImVec2(thick, avail.y));
    if (sx.active) split_x += sx.delta / body_w;
    ImGui::SameLine(0, 0);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::BeginChild("##right", ImVec2(right_w, avail.y), false, ImGuiWindowFlags_NoScrollbar);
    {
        ImVec2 ra = ImGui::GetContentRegionAvail();
        float col_h = ra.y - thick;
        if (col_h < 1.0f) col_h = 1.0f;
        const float min_zone = 60.0f;
        split_y = ImClamp(split_y, min_zone / col_h, 1.0f - min_zone / col_h);
        float top_h = ImFloor(col_h * split_y);
        float bot_h = col_h - top_h;

        ImGui::BeginChild("##rtop", ImVec2(ra.x, top_h), true, ImGuiWindowFlags_NoScrollbar);
        right_top.draw(ctx);
        ImGui::EndChild();

        SplitterState sy = imgui_splitter("##split_tb", SplitAxis::Y, ImVec2(ra.x, thick));
        if (sy.active) split_y += sy.delta / col_h;

        ImGui::BeginChild("##rbottom", ImVec2(ra.x, bot_h), true, ImGuiWindowFlags_NoScrollbar);
        right_bottom.draw(ctx);
        ImGui::EndChild();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::PopStyleVar();
    ImGui::End();
}

void Editor::draw_menu()
{
    if (ImGui::BeginMenu("Panels")) {
        for (auto& p : panels) ImGui::MenuItem(p->name(), nullptr, &p->open);
        ImGui::EndMenu();
    }
}

void Editor::take_screenshot(EditorContext& ctx)
{
    ctx.render_path->screenshot.requested = true;
}

}