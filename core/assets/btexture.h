#pragma once
#include <cstdint>

namespace lumen {

enum TextureFlags : uint32_t {
    TEXTURE_FLAG_SRGB = 1u << 0,
    TEXTURE_FLAG_NORMAL_MAP = 1u << 1,
};

struct BTexturePayloadHeader {
    uint32_t vk_format;
    uint32_t width;
    uint32_t height;
    uint32_t mip_count;
    uint32_t flags;
    uint32_t _pad;
};

static_assert(sizeof(BTexturePayloadHeader) == 24, "BTexturePayloadHeader layout changed");
static_assert(std::is_trivially_copyable_v<BTexturePayloadHeader>, "must be blittable");

}