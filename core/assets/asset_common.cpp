#include <core/assets/asset_common.h>
#include <toml++/toml.hpp>

namespace lumen {

AssetInfo read_asset_info(const std::filesystem::path& p_path)
{
    AssetInfo info;

    toml::table tbl;
    try { tbl = toml::parse_file(p_path.string()); }
    catch (const toml::parse_error&) { return info; }

    auto asset = tbl["asset"];
    info.version = static_cast<uint32_t>(asset["version"].value_or<int64_t>(0));
    if (auto g = asset["guid"].value<std::string>()) info.guid = Guid::from_string(*g);
    info.type = asset_type_from_u32(static_cast<uint32_t>(asset["type"].value_or<int64_t>(0)));

    return info;
}

bool read_asset_header(const std::filesystem::path& p_path, BAssetHeader& r_header)
{
    std::ifstream f(p_path, std::ios::binary);
    if (!f) return false;
    f.read(reinterpret_cast<char*>(&r_header), sizeof(r_header));
    if (!f || f.gcount() != (std::streamsize)sizeof(r_header)) return false;
    return r_header.magic == BCON_MAGIC && r_header.version == BASSET_VERSION;
}

}