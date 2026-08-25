#include <editor/asset_manager/asset_manager.h>
#include <drivers/imgui/imgui_helpers.h>
#include <core/project/project.h>
#include <editor/editor_resources.h>
#include <drivers/imgui/imgui_driver.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace ballistic {
    
Error AssetManager::initialize()
{
    return Error::Ok;
}

void AssetManager::shutdown()
{

}

void AssetManager::on_update(EditorContext& ctx)
{   
    if (selected_folder.empty()) selected_folder = ctx.project->assets_dir;
    const std::filesystem::path& root = ctx.project->assets_dir;

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

    const ImVec2 region_p0 = ImGui::GetCursorScreenPos();

    const float thick = 6.0f;
    ImVec2 avail = ImGui::GetContentRegionAvail();

    float body_w = avail.x - thick;
    if (body_w < 1.0f) body_w = 1.0f;
    const float min_side = 120.0f;
    split_x = ImClamp(split_x, min_side / body_w, 1.0f - min_side / body_w);
    float left_w = ImFloor(body_w * split_x);
    float right_w = body_w - left_w;

    ImGui::BeginChild("##left", ImVec2(left_w, avail.y), true);
    toolbar.draw_sidebar(root, selected_folder);
    ImGui::EndChild();

    ImGui::SameLine(0, 0);
    SplitterState sx = imgui_splitter("##am_split_lr", SplitAxis::X, ImVec2(thick, avail.y));
    if (sx.active) split_x += sx.delta / body_w;
    ImGui::SameLine(0, 0);

    ImGui::BeginChild("##right", ImVec2(right_w, avail.y), true);
    toolbar.draw_header(ctx, root, selected_folder, search_buf, sizeof(search_buf));
    ImGui::BeginChild("##bottom_right", ImVec2(0, 0), true);
    grid.draw(ctx, selected_folder, search_buf);
    ImGui::EndChild();
    ImGui::EndChild();

    _draw_divider_shadow(region_p0, avail.y, left_w);

    ImGui::End();
}

void AssetManager::_draw_divider_shadow(const ImVec2& region_p0, float region_h, float left_w)
{
    const float divider_x = region_p0.x + left_w;
    const float shadow_w  = 20.0f;
    const ImU32 c_edge = IM_COL32(0, 0, 0, 80);
    const ImU32 c_fade = IM_COL32(0, 0, 0, 0);
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilledMultiColor(ImVec2(divider_x - shadow_w, region_p0.y), ImVec2(divider_x, region_p0.y + region_h), c_fade, c_edge, c_edge, c_fade);
}

}