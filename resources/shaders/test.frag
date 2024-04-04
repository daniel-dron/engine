#version 430 core

out vec4 color;

layout (binding = 0) uniform sampler2D text;

in vec2 uvs;

void main() {
    color = vec4(texture(text, uvs).rgb, 1.0);
}