#include <editor/center_view/center_view.h>
#include <core/rendering/renderer.h>
#include <drivers/imgui/imgui_driver.h>
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
    
    RenderGraph::ImageResource* sel = ctx.renderer->graph.image_resource_by_id(selected_name_id);
    VkImageView sel_view = (sel && sel->image) ? sel->image->image_view : VK_NULL_HANDLE;
    VkDescriptorSet set = ctx.imgui->texture_cache.get(sel_view);

    if (set) {
        ImGui::Image((ImTextureID)set, size);
    } else {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(25, 25, 25, 255));
    }

    const float margin = 8.0f;
    ImVec2 button_size = ImVec2(24, 24);
    ImVec2 button_pos = ImVec2(ImGui::GetWindowContentRegionMax().x - button_size.x, ImGui::GetWindowContentRegionMin().y + margin);

    ImGui::SetCursorPos(button_pos);
    if (ImGui::Button(ICON_FA_ELLIPSIS, button_size)) {
        ImGui::OpenPopup("##viewport_source");
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("##viewport_source")) {
        if (ImGui::Selectable("Final Output", selected_name_id == 0)) selected_name_id = 0;
        ImGui::Separator();

        bool any = false;
        for (const RenderGraph::ImageResource& r : ctx.renderer->graph.image_resources) {
            if (r.image->state.layout != VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL || !r.image) continue;
            if (r.image_create_info.sizing != drivers::DeviceDriverVulkan::ImageCreateInfo::Sizing::ViewportRelative) continue;

            any = true;
            const std::string& name = ctx.renderer->graph.debug_names[r.name_id];
            bool is_selected = (r.name_id == selected_name_id);
            if (ImGui::Selectable(name.c_str(), is_selected)) selected_name_id = r.name_id;
        }
        if (!any) ImGui::TextDisabled("(no inspectable resources)");
        
        ImGui::Text("%llu", selected_name_id);

        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();
}

void CenterView::draw(EditorContext& ctx)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewportt", nullptr, ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

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

    ImGui::InvisibleButton("##vsplit", ImVec2(avail.x, handle_h));
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();
    if (hovered || active) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);


    if (active) {
        float dy = ImGui::GetIO().MouseDelta.y;
        if (debugger.collapsed) {
            if (dy < 0.0f) {
                debugger.collapsed = false;
                split_ratio = 1.0f - (min_content / usable);
            split_ratio = 0.6f;
            }
        } else {
            split_ratio += dy / usable;
            if (usable * (1.0f - split_ratio) < min_content) {
                debugger.collapsed = true;
                split_ratio = 0.5f;
            }
        }
    }
    
    
    ImVec2 bmin = ImGui::GetItemRectMin();
    ImVec2 bmax = ImGui::GetItemRectMax();
    float cy = ImFloor((bmin.y + bmax.y) * 0.5f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    
    const float grip_w = 40.0f;
    float gx = ImFloor((bmin.x + bmax.x) * 0.5f);
    ImU32 grip_col = ImGui::GetColorU32(active ? ImGuiCol_SeparatorActive : hovered ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
    dl->AddRectFilled(ImVec2(gx - grip_w * 0.5f, cy - 2.0f), ImVec2(gx + grip_w * 0.5f, cy + 2.0f), grip_col, 2.0f);

    if (!debugger.collapsed) {
        ImGui::BeginChild("##bottom", ImVec2(avail.x, content_h), true, ImGuiWindowFlags_NoScrollbar);
        debugger.draw_content(ctx);
        ImGui::EndChild();
    }

    debugger.draw_strip(ctx);

    ImGui::PopStyleVar();
    ImGui::End();
}
    
}