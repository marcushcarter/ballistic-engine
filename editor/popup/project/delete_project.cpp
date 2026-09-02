#include <editor/popup/project/delete_project.h>
#include <editor/project_manager/project_manager.h>
#include <core/project/project.h>
#include <core/io/path.h>
#include <imgui.h>

namespace lumen {

void DeleteProjectPopup::before_begin()
{
    ImGui::SetNextWindowSize(ImVec2(500, 125), ImGuiCond_Appearing);
}

void DeleteProjectPopup::draw_contents(EditorContext&)
{
    ImGui::Text("Permanently delete project \"%s\"?", project_name.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("%s", project_path.string().c_str());
    ImGui::TextColored(ImVec4(0.86f, 0.35f, 0.35f, 1.0f), "This erases the project folder from disk and cannot be undone.");
}

void DeleteProjectPopup::draw_footer(EditorContext& ctx) {
    const char* labels[] = { "Delete", "Cancel" };
    switch (footer_buttons(labels, 2)) {
        case 0:
            if (Project::destroy(project_path) == Error::Ok) {
                auto& recent = ctx.project_manager->recent;
                for (size_t i = 0; i < recent.size(); ++i)
                    if (recent[i].path == project_path) { recent.erase(recent.begin() + i); break; }
                ctx.project_manager->save_recents();
                ctx.project_manager->selected = -1;
            }
            close();
            break;
        case 1: close(); break;
    }
}

}