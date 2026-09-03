#include <editor/docking/center_view/overlay_bar.h>
#include <drivers/imgui/imgui_helpers.h>
#include <imgui.h>
#include <cstdio>

namespace lumen {

static void overlay_push_style()
{
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(28, 28, 30, 130));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(60, 62, 66, 190));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(80, 82, 88, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 999.0f);
}

void OverlayBar::begin(ImVec2 p_origin, ImVec2 p_region, Align p_align, float p_margin, float p_spacing)
{
    origin = p_origin;
    region = p_region;
    align = p_align;
    margin = p_margin;
    spacing = p_spacing;
    row_y = origin.y + margin;
    cursor_x = (align == Align::Left) ? origin.x + margin : origin.x + region.x - margin;

    ImVec4 ac = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
    ac.w = 130.0f / 255.0f;
    active_col = ImGui::GetColorU32(ac);

    overlay_push_style();
}

void OverlayBar::end()
{
    ImGui::PopStyleVar(1);
    ImGui::PopStyleColor(3);
}

bool OverlayBar::_emit(const char* p_label, ImVec2 p_size, bool p_active)
{
    const ImVec2 fp = ImGui::GetStyle().FramePadding;
    float w = p_size.x, h = p_size.y;
    if (w <= 0.0f) w = ImGui::CalcTextSize(p_label, nullptr, true).x + fp.x * 2.0f;
    if (h <= 0.0f) h = ImGui::GetFrameHeight();

    float x;
    if (align == Align::Left) {
        x = cursor_x;
        cursor_x += w + spacing;
    } else {
        cursor_x -= w;
        x = cursor_x;
        cursor_x -= spacing;
    }

    ImGui::SetCursorScreenPos(ImVec2(x, row_y));

    int pushed = 0;
    if (p_active) { ImGui::PushStyleColor(ImGuiCol_Button, active_col); ++pushed; }
    const bool clicked = ImGui::Button(p_label, ImVec2(w, h));
    if (pushed) ImGui::PopStyleColor(pushed);
    return clicked;

}

bool OverlayBar::button(const char* p_label, ImVec2 p_size)
{
    return _emit(p_label, p_size, false);
}

bool OverlayBar::toggle(const char* p_label, bool& p_active, ImVec2 p_size)
{
    const bool clicked = _emit(p_label, p_size, p_active);
    if (clicked) p_active = !p_active;
    return clicked;
}

bool OverlayBar::combo(const char* p_id, const char* p_preview, float p_width)
{
    float x;
    if (align == Align::Left) {
        x = cursor_x;
        cursor_x += p_width + spacing;
    } else {
        cursor_x -= p_width;
        x = cursor_x;
        cursor_x -= spacing;
    }

    ImGui::SetCursorScreenPos(ImVec2(x, row_y));
    ImGui::SetNextItemWidth(p_width);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(28, 28, 30, 130));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(60, 62, 66, 190));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(80, 82, 88, 220));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(24, 24, 26, 255));
    const bool open = ImGui::BeginCombo(p_id, p_preview, ImGuiComboFlags_HeightLargest);
    ImGui::PopStyleColor(4);
    return open;
}

bool OverlayBar::begin_menu(const char* p_label, ImVec2 p_size)
{
    const ImVec2 fp = ImGui::GetStyle().FramePadding;
    float w = p_size.x, h = p_size.y;
    if (w <= 0.0f) w = ImGui::CalcTextSize(p_label, nullptr, true).x + fp.x * 2.0f;
    if (h <= 0.0f) h = ImGui::GetFrameHeight();

    float x;
    if (align == Align::Left) {
        x = cursor_x;
        cursor_x += w + spacing;
    } else {
        cursor_x -= w;
        x = cursor_x;
        cursor_x -= spacing;
    }

    ImGui::SetCursorScreenPos(ImVec2(x, row_y));

    int pushed = 0;
    if (ImGui::IsPopupOpen(p_label)) { ImGui::PushStyleColor(ImGuiCol_Button, active_col); ++pushed; }
    const bool clicked = ImGui::Button(p_label, ImVec2(w, h));
    if (pushed) ImGui::PopStyleColor(pushed);

    const ImVec2 bmin = ImGui::GetItemRectMin();
    const ImVec2 bmax = ImGui::GetItemRectMax();
    if (clicked) ImGui::OpenPopup(p_label);

    ImGui::SetNextWindowPos(ImVec2(bmin.x, bmax.y + 2.0f));

    ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(24, 24, 26, 255));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(60, 62, 66, 190));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(80, 82, 88, 220));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

    if (ImGui::BeginPopup(p_label)) {
        return true;
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
    return false;
}

void OverlayBar::end_menu()
{
    ImGui::EndPopup();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(3);
}

void OverlayBar::gap(float p_w)
{
    if (align == Align::Left) cursor_x += p_w;
    else cursor_x -= p_w;
}

}