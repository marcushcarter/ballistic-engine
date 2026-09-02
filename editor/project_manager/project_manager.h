#pragma once
#include <editor/editor_context.h>
#include <core/base/error.h>
#include <filesystem>
#include <string>
#include <vector>

namespace lumen {

struct ProjectManager
{
    struct Entry {
        std::string name;
        std::filesystem::path path;
        bool favorite = false;
        std::string last_opened;
    };

    std::vector<Entry> recent;
    int selected = -1;
    int  sort_index = 0;
    char filter_buf[128] = {};

    Error initialize();
    void shutdown();

    void load_recents();
    void save_recents();
    void add_recent(const std::filesystem::path& p_root, std::string_view p_name);

    void _draw_list(EditorContext& ctx);

    void on_update(EditorContext& ctx);
};

}