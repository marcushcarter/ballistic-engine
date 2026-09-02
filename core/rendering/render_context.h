#pragma once

namespace lumen {

struct RenderGraph;
namespace drivers { struct DeviceDriverVulkan; struct ImGuiDriver; }

struct RenderContext
{
    drivers::DeviceDriverVulkan* dd = nullptr;
    drivers::ImGuiDriver* imgui = nullptr;
    RenderGraph* graph = nullptr;
};

}