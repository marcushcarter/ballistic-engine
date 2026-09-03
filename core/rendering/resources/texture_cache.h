#pragma once
#include <drivers/vulkan/device_driver_vulkan.h>
#include <core/assets/guid.h>
#include <core/base/error.h>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <filesystem>

namespace lumen {

struct LTexture {
    Guid guid;
    drivers::DeviceDriverVulkan::Image image;
};

struct TextureCache
{
    drivers::DeviceDriverVulkan* dd = nullptr;

    std::vector<LTexture> slots;
    std::vector<uint32_t> free_slots;
    std::unordered_map<Guid, uint32_t, GuidHash, GuidEq> by_guid;

    Error initialize(drivers::DeviceDriverVulkan& r_dd);
    
    uint32_t load(Guid p_guid, const std::filesystem::path& p_path);
    void unload(Guid p_guid);
    void clear();

    const LTexture* get(Guid p_guid) const {
        auto it = by_guid.find(p_guid);
        return it == by_guid.end() ? nullptr : &slots[it->second];
    }

    uint32_t bindless_of(Guid p_guid) const {
        const LTexture* t = get(p_guid);
        return t ? t->image.bindless_sampled : UINT32_MAX;
    }
};
    
}