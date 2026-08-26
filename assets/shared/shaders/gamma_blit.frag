#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(set = 0, binding = 0) uniform texture2D uTextures[];
// layout(set = 0, binding = 1) uniform image2D uStorageImages[];
layout(set = 0, binding = 2) uniform sampler uSamplers[];

layout(push_constant) uniform PC {
    uint srcIndex;
    uint samplerIndex;
} pc;

vec4 sample_bindless(uint tex_id, uint sampler_id, vec2 uv) {
    return texture(sampler2D(uTextures[nonuniformEXT(tex_id)], uSamplers[nonuniformEXT(sampler_id)]), uv);
}

vec3 linear_to_srgb(vec3 color) {
    return mix(color * 12.92, 1.055 * pow(color, vec3(1.0 / 2.4)) - 0.055, step(0.0031308, color));
}

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 oColor;

void main()
{
    vec2 flipped_uv = vec2(vUV.x, 1.0 - vUV.y);
    vec4 color = sample_bindless(pc.srcIndex, pc.samplerIndex, flipped_uv);
    color.rgb = linear_to_srgb(color.rgb);
    oColor = color;
}