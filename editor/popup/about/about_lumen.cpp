#include <editor/popup/about/about_lumen.h>
#include <editor/editor_resources.h>
#include <drivers/imgui/imgui_driver.h>
#include <drivers/imgui/imgui_helpers.h>
#include <core/version.h>

namespace lumen {
    
void AboutLumenPopup::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);
}

void AboutLumenPopup::draw_contents(EditorContext& ctx)
{
    constexpr float header_height = 90.0f;

    ImGui::BeginChild("AboutHeader", ImVec2(0, header_height), false);
    {
        VkDescriptorSet logo_set = ctx.imgui->texture_cache.get(ctx.resources->logo_image.image_view);
        float scale = header_height / ctx.resources->logo_image.extent.height;
        ImGui::Image(logo_set, ImVec2((float)ctx.resources->logo_image.extent.width * scale, (float)ctx.resources->logo_image.extent.height * scale));
    
        ImGui::SameLine();
        ImGui::BeginGroup();
        imgui_link("Lumen Engine", "https://github.com/marcushcarter/lumen");
        ImGui::Text("Version %s", LUMEN_VERSION_FULL_BUILD);
        ImGui::EndGroup();
    }
    ImGui::EndChild();

    if (ImGui::BeginTabBar("AboutTabs")) {
        if (ImGui::BeginTabItem("License")) {
            ImGui::BeginChild("LicenseContent", ImVec2(0, 0), true);
            ImGui::TextWrapped(ctx.resources->license_text.c_str());
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("EULA")) {
            ImGui::BeginChild("EULAContent", ImVec2(0, 0), true);
            ImGui::TextWrapped(ctx.resources->eula_text.c_str());
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

}