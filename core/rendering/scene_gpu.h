#pragma once
#include <cstdint>
#include <glm/glm.hpp>

namespace lumen {
    
using namespace glm;

static constexpr uint32_t MAX_INSTANCES = 64u * 1024;

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

struct Instance {
    uint32_t mesh_id;
    uint32_t transform_id;
    uint32_t _pad0[2];
};

struct Transform {
    mat4 prev_mtx;
    mat4 curr_mtx;
};

struct IndirectDispatch { uint32_t x, y, z; };

}