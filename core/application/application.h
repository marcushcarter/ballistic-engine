#pragma once
#include <drivers/windows/window_driver_win32.h>
#include <drivers/vulkan/device_driver_vulkan.h>
#include <drivers/vulkan/context_driver_vulkan.h>
#include <drivers/imgui/imgui_driver.h>
#include <core/rendering/renderer.h>
#include <core/project/project.h>
#include <core/scene/scenes.h>
#include <core/base/tasks.h>
#include <core/base/error.h>
#include <string>
#include <filesystem>

namespace lumen {

struct RenderPath;

struct ApplicationCreateInfo
{
    std::string window_title;
    int width = 1280;
    int height = 720;
    const char* ini_path = nullptr;
};

struct Application
{
    drivers::WindowDriverWin32 win32;

    drivers::ContextDriverVulkan cd;
    drivers::DeviceDriverVulkan dd;
    Renderer renderer;
    RenderPath* render_path = nullptr;
    RenderPath* pending_render_path = nullptr;

    TaskSystem tasks;

    drivers::ImGuiDriver imgui;

    Scenes scenes;

    Project project;

    Error initialize(const ApplicationCreateInfo& p_initialize_info);
    void shutdown();
    int run();

    Error project_load(const std::filesystem::path &p_root);
    void project_unload();
    
    void render_path_request(RenderPath* p_next);
    void _apply_pending_render_path();

    virtual Error on_init() = 0;
    virtual void on_update(float p_dt) = 0;
    virtual void on_shutdown() = 0;
    
    virtual bool wants_docking() const { return false; }
    virtual bool wants_custom_titlebar() const { return false; }

    virtual RenderPath* create_render_path() = 0;

    virtual ~Application() = default;
};

}