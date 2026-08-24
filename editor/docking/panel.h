#pragma once
#include <editor/editor_context.h>
#include <imgui.h>

namespace ballistic {

enum class DockZone { Left, RightTop, RightBottom };

inline const char* to_string(DockZone z) {
    switch (z) {
        case DockZone::Left: return "Left";
        case DockZone::RightTop: return "RightTop";
        case DockZone::RightBottom: return "RightBottom";
    }
    return "RightBottom";
}

inline DockZone dock_zone_from_string(std::string_view s, DockZone fallback) {
    if (s == "Left") return DockZone::Left;
    if (s == "RightTop") return DockZone::RightTop;
    if (s == "RightBottom") return DockZone::RightBottom;
    return fallback;
}

struct Panel
{
    bool open = true;
    DockZone zone = DockZone::RightBottom;

    virtual ~Panel() = default;
    virtual const char* name() const = 0;
    virtual DockZone default_zone() const { return DockZone::RightBottom; }

    virtual void draw_contents(EditorContext& ctx) = 0;
    virtual ImGuiWindowFlags window_flags() const { return 0; }
    virtual int push_style() { return 0; }
    virtual void before_begin() {}
};

}