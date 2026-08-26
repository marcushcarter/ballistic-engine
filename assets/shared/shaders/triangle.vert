#version 450

layout(push_constant) uniform Push {
    mat4 viewProj;
} pc;

layout(location = 0) out vec3 vColor;

vec2 positions[3] = vec2[](vec2(0.0, 0.5), vec2(-0.5, -0.5), vec2(0.5, -0.5));
vec3 colors[3] = vec3[](vec3(1,0,0), vec3(0,1,0), vec3(0,0,1));

void main() {
    gl_Position = pc.viewProj * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    vColor = colors[gl_VertexIndex];
}