#version 450

layout(location = 0) in  vec3 vColor;

layout(location = 0) out vec4 oAlbedo;

void main() {
    oAlbedo = vec4(vColor, 1.0);
}