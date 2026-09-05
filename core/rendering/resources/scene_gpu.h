#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace lumen {
    
using namespace glm;

static constexpr uint32_t MAX_INSTANCES = 64u * 1024;

struct MeshInstance {
    uint32_t mesh_id;
    uint32_t mtx_id;
    uint32_t vis_base;
    uint32_t slot_table_base;
};

struct IndirectDispatch {
    uint32_t x, y, z;
};

struct CameraUniform {
    mat4 view;
    mat4 proj;
    mat4 view_proj;
    mat4 inv_view;
    mat4 inv_proj;
    mat4 inv_view_proj;

    vec4 position;
    vec4 frustum_planes[6];

    float near_z;
    float far_z;
    float fov_y;
    float aspect;
};

}