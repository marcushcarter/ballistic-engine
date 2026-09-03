#include <editor/assets/mesh_cooker.h>
#include <editor/editor_context.h>
#include <editor/assets/asset_import_tracker.h>
#include <editor/assets/write_atomic.h>
#include <core/assets/asset_common.h>
#include <core/assets/lmesh.h>
#include <core/project/project.h>
#include <core/base/tasks.h>
#include <drivers/toml/toml_helpers.h>
// #include <vulkan/vulkan.h>
// #include <rdo_bc_encoder.h>
#include <utils.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <cmath>

namespace lumen {

Error MeshCooker::_cook(const Job& p_job)
{
    using enum Error;

    auto report = [&](float v){ if (p_job.progress) p_job.progress->progress.store(v, std::memory_order_relaxed); };
    report(0.0f);

    // load mesh with assimp
        
    report(0.1f);

    bool skinned = false;
    // setup mesh data (clusters)

    // push mesh data
    for (uint32_t i = 0; i < 100000000; i++) {
        i = i;
        report(0.1f + 0.8f * (float)(i + 1) / (float)100000000);
    }

    std::vector<uint8_t> blocks;
//     for (uint32_t m = 0; m < mip_count; ++m) {
//         const uint32_t mw = dims[m].first, mh = dims[m].second;
//         utils::image_u8 src(mw, mh);
//         const std::vector<uint8_t>& buf = mips[m];
//         for (uint32_t y = 0; y < mh; ++y) for (uint32_t x = 0; x < mw; ++x) {
//             const uint8_t* px = &buf[(static_cast<size_t>(y) * mw + x) * 4];
//             src(x, y) = utils::color_quad_u8(px[0], px[1], px[2], px[3]);
//         }
//         rdo_bc::rdo_bc_params p;
//         p.m_dxgi_format = fmt.dxgi;
//         p.m_bc7_uber_level = 4; // move to 6 to have better compression but takes longer (around 4x)
//         p.m_perceptual = fmt.srgb;
//         p.m_rdo_lambda = 0.3f;
//         p.m_rdo_multithreading = true;
//         p.m_status_output = false;
//         if (fmt.bc45) { p.m_bc45_channel0 = 0; p.m_bc45_channel1 = 1; }

//         rdo_bc::rdo_bc_encoder enc;
//         if (!enc.init(src, p)) return Failed;
//         if (!enc.encode()) return Failed;
//         const uint8_t* b = static_cast<const uint8_t*>(enc.get_blocks());
//         blocks.insert(blocks.end(), b, b + enc.get_total_blocks_size_in_bytes());

//         report(0.1f + 0.8f * (float)(m + 1) / (float)mip_count);
//     }

    LMeshPayloadHeader ph{};
//     ph.vk_format = fmt.vk;
//     ph.width = width;
//     ph.height = height;
//     ph.mip_count = mip_count;
    ph.flags = skinned ? MESH_FLAG_SKINNED : 0u;

    LAssetHeader ah{};
    ah.magic = BCON_MAGIC;
    ah.version = LASSET_VERSION;
    ah.guid = p_job.guid;
    ah.type = AssetType::Mesh;
    ah.payload_size = static_cast<uint32_t>(sizeof(ph) + blocks.size());

    std::vector<uint8_t> bin;
    bin.reserve(sizeof(ah) + sizeof(ph) + blocks.size());
    auto append = [&](const void* d, size_t n){ const uint8_t* p = static_cast<const uint8_t*>(d); bin.insert(bin.end(), p, p + n); };
    append(&ah, sizeof(ah));
    append(&ph, sizeof(ph));
    append(blocks.data(), blocks.size());

    std::error_code ec;
    std::filesystem::create_directories(p_job.content_bin.parent_path(), ec);
    if (!write_file_atomic(p_job.content_bin, bin.data(), bin.size())) return Failed;

    report(0.95f);

    toml::table tbl {
        { "asset", toml::table{
            { "version", static_cast<int64_t>(LASSET_VERSION) },
            { "guid", p_job.guid.to_string() },
            { "type", static_cast<int64_t>(AssetType::Mesh) } }},
        { std::string(asset_type_section(AssetType::Mesh)), toml::table{
            { "path", p_job.source.generic_string() },
            { "vertices", static_cast<int64_t>(0) },
            { "triangles", static_cast<int64_t>(0) },
            { "clusters", static_cast<int64_t>(0) },
            { "skinned", skinned },
            { "dag", p_job.settings.dag },
            }},
    };
    std::string text; { std::ostringstream ss; ss << tbl; text = ss.str(); }
    if (!write_file_atomic(p_job.dst_lmesh, text.data(), text.size())) return Failed;

    report(1.0f);
    return Ok;
}

Error MeshCooker::import(const Project& p_project, const std::filesystem::path& p_src, const std::filesystem::path& p_dst, Guid& r_guid, const CookSettings& p_settings)
{
    using enum Error;
    Job job;
    job.source = p_src;
    job.dst_lmesh = p_dst;
    job.settings = p_settings;
    if (AssetImportTracker::resolve_import(p_project, AssetType::Mesh, p_dst, job.guid, job.content_bin) != Ok) return Failed;
    if (_cook(job) != Ok) return Failed;
    r_guid = job.guid;
    return Ok;   
}

void MeshCooker::import_async(EditorContext& ctx, const std::filesystem::path& p_src, const std::filesystem::path& p_dst, const MeshCooker::CookSettings& p_settings)
{
    Job job;
    job.source = p_src;
    job.dst_lmesh = p_dst;
    job.settings = p_settings;
    if (AssetImportTracker::resolve_import(*ctx.project, AssetType::Mesh, p_dst, job.guid, job.content_bin) != Error::Ok) {
        log_write("Mesh import failed: %s", p_src.string().c_str());
        return;
    }
    job.progress = ctx.imports->add(p_dst, job.guid, job.content_bin);
    ctx.tasks->dispatch([job]{
        if (MeshCooker::_cook(job) != Error::Ok && job.progress && !job.progress->cancel.load(std::memory_order_relaxed)) job.progress->progress.store(1.0f, std::memory_order_relaxed);
    });
}

}