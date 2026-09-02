#pragma once
#include <editor/editor_context.h>
#include <imgui.h>

namespace lumen {

struct Popup
{
    bool open = false;

    virtual ~Popup() = default;
    virtual const char* name() const = 0;
    void close() { open = false; ImGui::CloseCurrentPopup(); }

    void draw(EditorContext& ctx)
    {
        if (open) {
            ImGui::OpenPopup(name());
            on_open(ctx);
            open = false;
        }

        int style_count = push_style();
        before_begin();        
        ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal(name(), nullptr, window_flags())) {
            const float footer_h = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y;
            ImGui::BeginChild("##popup_body", ImVec2(0, -footer_h));
            draw_contents(ctx);
            ImGui::EndChild();
            draw_footer(ctx);
            ImGui::EndPopup();
        }
        if (style_count) ImGui::PopStyleVar(style_count);
    }

    virtual void on_open(EditorContext&) {}
    virtual int push_style() { return 0; }
    virtual void before_begin() { ImGui::SetNextWindowSize(ImVec2(500, 250), ImGuiCond_Appearing); }
    virtual ImGuiWindowFlags window_flags() const { return ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings; }
    virtual void draw_contents(EditorContext&) {}

    virtual void draw_footer(EditorContext&) {
        const char* labels[] = { "Close" };
        switch (footer_buttons(labels, 1)) { case 0: close(); break; }
    }

    static int footer_buttons(const char* const* labels, int count, float bw = 120.0f, unsigned disabled_mask = 0) {
        ImGuiStyle& s = ImGui::GetStyle();
        float total = count * bw + s.ItemSpacing.x * (count - 1);
        float avail = ImGui::GetContentRegionAvail().x;
        if (total < avail) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - total) * 0.5f);
        int clicked = -1;
        for (int i = 0; i < count; ++i) {
            if (i) ImGui::SameLine();
            ImGui::PushID(i);
            bool dis = (disabled_mask >> i) & 1u;
            if (dis) ImGui::BeginDisabled();
            if (ImGui::Button(labels[i], ImVec2(bw, 0))) clicked = i;
            if (dis) ImGui::EndDisabled();
            ImGui::PopID();
        }
        return clicked;
    }
};

}