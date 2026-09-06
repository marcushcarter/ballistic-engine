#ifndef CAMERA_GLSL
#define CAMERA_GLSL

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(buffer_reference, scalar) readonly buffer CameraBuffer {
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

#endif