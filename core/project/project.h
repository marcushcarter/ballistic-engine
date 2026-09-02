#pragma once
#include <core/project/project_settings.h>
#include <core/base/error.h>
#include <core/assets/guid.h>
#include <filesystem>
#include <string>

namespace lumen {

struct Project
{
    static constexpr uint32_t FORMAT_VERSION = 1;
    
    static constexpr const char* FILE_NAME = "Data/project.config";
    static constexpr const char* FILE_ASSETDB = "Data/assetdb.bin";

    static constexpr const char* DIR_DATA = "Data";
    static constexpr const char* DIR_ASSETS = "Data/Assets";
    static constexpr const char* DIR_CONTENT = "Data/Content";

    std::filesystem::path root;
    std::filesystem::path data_dir;
    std::filesystem::path assets_dir;
    std::filesystem::path content_dir;

    std::string name;

    ProjectSettings settings;

    void _resolve_dirs(const std::filesystem::path& p_root);
    static Error _ensure_layout(const std::filesystem::path& p_root);

    Error load(const std::filesystem::path& p_root);
    Error save() const;
    void unload();

    static Error create(const std::filesystem::path& p_root, std::string_view p_name);
    static Error destroy(const std::filesystem::path& p_root);

    static std::string peek_name(const std::filesystem::path& p_root);

    // Turns a serialized resources guid into a path turns the first two characters into the folder name so that we dont have thousands of binary files all in the same folder.
    std::filesystem::path content_path(Guid p_guid) const;

    bool loaded() const { return !root.empty(); }
};

}