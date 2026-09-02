#pragma once
#include <core/assets/guid.h>
#include <cstdint>
#include <string_view>
#include <filesystem>

namespace lumen {

enum class AssetType : uint32_t {
    None = 0,
    Texture = 1,
};

inline constexpr uint32_t BASSET_VERSION = 1;

inline AssetType asset_type_from_u32(uint32_t p_v) {
    switch (p_v) {
        case static_cast<uint32_t>(AssetType::Texture):  return AssetType::Texture;
        default: return AssetType::None;
    }
}

inline std::string_view asset_type_section(AssetType p_type) {
    switch (p_type) {
        case AssetType::Texture: return "texture";
        default: return "None";
    }
}

struct AssetInfo {
    uint32_t version = 0;
    Guid guid{};
    AssetType type = AssetType::None;
    bool valid() const { return version != 0 && type != AssetType::None; }
};

inline constexpr uint32_t BCON_MAGIC = uint32_t('B') | (uint32_t('C') << 8) | (uint32_t('O') << 16) | (uint32_t('N') << 24);

struct BAssetHeader {
    uint32_t magic;
    uint32_t version;
    Guid guid;
    AssetType type;
    uint32_t payload_size;
};
static_assert(sizeof(Guid) == 8, "header assumes 8-byte Guid");
static_assert(sizeof(BAssetHeader) == 24, "BAssetHeader layout changed");

AssetInfo read_asset_info(const std::filesystem::path& p_path);
bool read_asset_header(const std::filesystem::path& p_path, BAssetHeader& r_header);

}