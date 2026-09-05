#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace lumen {

using namespace glm;

enum MeshFlags : uint32_t {
    MESH_FLAG_SKINNED = 1u << 0,
};

struct Vertex {
    u16vec3 position;
    i16vec2 normal;
    u16vec2 uv;
};

struct SkinVertex {
    u8vec4 joints;
    u8vec4 weights;
};

static constexpr uint32_t BVH_LEAF_BIT = 0x80000000u;

struct BVHNode {
    vec3 bounds_min;
    uint32_t left;
    vec3 bounds_max;
    uint32_t right;
};

struct Cluster {
    uint32_t index_base;
    uint32_t index_count;
    vec4 cull_sphere, cull_cone;
    vec4 lod_sphere, lod_parent_sphere;
    float lod_error, lod_parent_error;
};

struct LMeshPayloadHeader {
    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t tri_count;
    uint32_t slot_table_count;
    uint32_t cluster_count;
    uint32_t bvh_node_count;
    uint32_t flags;
    vec3 pos_min, pos_extent;
    vec2 uv_min, uv_extent;
    vec4 bounds_sphere;
};

static_assert(sizeof(LMeshPayloadHeader) == 84, "LMeshPayloadHeader layout changed");
static_assert(std::is_trivially_copyable_v<LMeshPayloadHeader>, "must be blittable");

}