#include <editor/docking/asset_browser/asset_browser.h>
#include <core/project/project.h>
#include <editor/editor_resources.h>
#include <drivers/imgui/imgui_driver.h>

namespace ballistic {

void AssetBrowserPanel::_draw_divider_shadow(const ImVec2& region_p0, float region_h)
{
    // Shadow.
    {
        const float divider_x = region_p0.x + left_width;
        const float shadow_w  = 20.0f;
        const ImU32 c_edge = IM_COL32(0, 0, 0, 80);
        const ImU32 c_fade = IM_COL32(0, 0, 0, 0);
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        dl->AddRectFilledMultiColor(ImVec2(divider_x - shadow_w, region_p0.y), ImVec2(divider_x, region_p0.y + region_h), c_fade, c_edge, c_edge, c_fade);
    }
}

void AssetBrowserPanel::draw_contents(EditorContext& ctx)
{
    if (selected_folder.empty()) selected_folder = ctx.project->assets_dir;
    
    const std::filesystem::path& root = ctx.project->assets_dir;

    const ImVec2 region_p0 = ImGui::GetCursorScreenPos();
    const float region_h = ImGui::GetContentRegionAvail().y;

    ImGui::BeginChild("##left", ImVec2(left_width, 0), true);
    toolbar.draw_sidebar(root, selected_folder);
    ImGui::EndChild();
    
    ImGui::SameLine(0.0f, 0.0f);

    ImGui::BeginChild("##right", ImVec2(0, 0), true);
    {
        toolbar.draw_header(ctx, root, selected_folder, search_buf, sizeof(search_buf));
        
        ImGui::BeginChild("##bottom_right", ImVec2(0, 0), true);
        grid.draw(ctx, selected_folder, search_buf);
        ImGui::EndChild();
    }
    ImGui::EndChild();

    _draw_divider_shadow(region_p0, region_h);
}

}