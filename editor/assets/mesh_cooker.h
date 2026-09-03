#pragma once
#include <editor/assets/asset_import_tracker.h>
#include <filesystem>

namespace lumen {

struct Project;
struct EditorContext;

struct MeshCooker
{
    struct CookSettings { bool dag = true; };

    struct Job {
        std::filesystem::path source;
        std::filesystem::path dst_lmesh;
        std::filesystem::path content_bin;
        Guid guid;
        CookSettings settings;
        std::shared_ptr<ImportControl> progress;
    };
    
    static Error _cook(const Job& p_job);

    static Error import(const Project& p_project, const std::filesystem::path& p_src, const std::filesystem::path& p_dst, Guid& r_guid, const CookSettings& p_settings = {});
    static void import_async(EditorContext& ctx, const std::filesystem::path& p_src, const std::filesystem::path& p_dst, const CookSettings& p_settings = {});
};

}