#include <editor/docking/center_view/center_view.h>
#include <drivers/imgui/imgui_driver.h>
#include <drivers/imgui/imgui_helpers.h>
#include <core/rendering/renderer.h>
#include <IconsFontAwesome6.h>
#include <imgui_internal.h>

namespace ballistic {

void CenterView::initialize()
{
    debugger.initialize();
}

void CenterView::_draw_scene(EditorContext& ctx)
{
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    if (!ImGui::IsAnyItemActive()) {
        ctx.renderer->request_size((uint32_t)size.x, (uint32_t)size.y);
    }

    if (!source_resolved) {
        for (const auto& [id, name] : ctx.renderer->graph.debug_names) {
            if (name == "Out_Color") {
                selected_name_id = id;
                source_resolved = true;
                break;
            }
        }
    }
    
    RenderGraph::ImageResource* sel = ctx.renderer->graph.image_resource_by_id(selected_name_id);
    VkImageView sel_view = (sel && sel->image) ? sel->image->image_view : VK_NULL_HANDLE;
    VkDescriptorSet set = ctx.imgui->texture_cache.get(sel_view);

    if (set) {
        ImGui::Image((ImTextureID)set, size);
    } else {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(25, 25, 25, 255));
    }

    left_overlay.begin(pos, size, OverlayBar::Align::Left);
    if (left_overlay.button(ICON_FA_BARS)) {}
    if (left_overlay.button(ICON_FA_CUBE " Perspective")) {}
    if (left_overlay.button(ICON_FA_ADDRESS_BOOK " Lit")) {}
    if (left_overlay.button("Show")) {}
    left_overlay.end();

    const char* src_label = "(no source)";
    if (selected_name_id != 0) {
        auto it = ctx.renderer->graph.debug_names.find(selected_name_id);
        if (it != ctx.renderer->graph.debug_names.end()) src_label = it->second.c_str();
    }

    // static bool gizmo_rotate = false, gizmo_move = false;
    right_overlay.begin(pos, size, OverlayBar::Align::Right);
    if (right_overlay.combo("##viewport_source", src_label, 160.0f)) {
        bool any = false;
        for (const RenderGraph::ImageResource& r : ctx.renderer->graph.image_resources) {
            if (!r.image || r.image->state.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) continue;
            if (r.image_create_info.sizing != drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative) continue;
            any = true;
            const std::string& name = ctx.renderer->graph.debug_names[r.name_id];
            if (ImGui::Selectable(name.c_str(), r.name_id == selected_name_id)) selected_name_id = r.name_id;
        }
        if (!any) ImGui::TextDisabled("(no inspectable resources)");

        ImGui::EndCombo();
    }
    // right_overlay.toggle(ICON_FA_ARROWS_ROTATE "##rot", gizmo_rotate, ImVec2(24,24));
    // right_overlay.toggle(ICON_FA_UP_DOWN_LEFT_RIGHT "##mov", gizmo_move, ImVec2(24,24));
    right_overlay.end();
}

void CenterView::draw(EditorContext& ctx)
{
    ImVec2 avail = ImGui::GetContentRegionAvail();
    const float strip_h = ImGui::GetFrameHeight();
    const float handle_h = 6.0f;
    float above_h = avail.y - strip_h;
    if (above_h < 1.0f) above_h = 1.0f;

    float usable = above_h - handle_h;
    if (usable < 1.0f) usable = 1.0f;

    const float min_scene = 80.0f;
    const float min_content = strip_h;

    float scene_h, content_h;
    if (debugger.collapsed) {
        scene_h = usable;
        content_h = 0.0f;
    } else {
        float min_r = min_scene / usable;
        float max_r = 1.0f - (min_content / usable);
        if (max_r < min_r) max_r = min_r;
        split_ratio = ImClamp(split_ratio, min_r, max_r);
        scene_h = ImFloor(usable * split_ratio);
        content_h = usable - scene_h;
    }
    
    ImGui::BeginChild("##top", ImVec2(avail.x, scene_h), false, ImGuiWindowFlags_NoScrollbar);
    _draw_scene(ctx);
    ImGui::EndChild();
    
    SplitterState s = imgui_splitter("##vsplit", SplitAxis::Y, ImVec2(avail.x, handle_h));
    if (s.active) {
        if (debugger.collapsed && s.activated) split_ratio = 1.0f;
        split_ratio += s.delta / usable;
        debugger.collapsed = (usable * (1.0f - split_ratio) < min_content);
    }
    
    ImVec2 bmin = ImGui::GetItemRectMin();
    ImVec2 bmax = ImGui::GetItemRectMax();
    float cy = ImFloor((bmin.y + bmax.y) * 0.5f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    
    const float grip_w = 40.0f;
    float gx = ImFloor((bmin.x + bmax.x) * 0.5f);
    ImU32 grip_col = ImGui::GetColorU32(s.active ? ImGuiCol_SeparatorActive : s.hovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
    dl->AddRectFilled(ImVec2(gx - grip_w * 0.5f, cy - 2.0f), ImVec2(gx + grip_w * 0.5f, cy + 2.0f), grip_col, 2.0f);

    if (!debugger.collapsed) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8)); // child padding, killed by the layout's 0-wrap
        ImGui::BeginChild("##bottom", ImVec2(avail.x, content_h), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));   // normal spacing inside the debugger
        debugger.draw_content(ctx);
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    debugger.draw_strip(ctx);
}
    
}