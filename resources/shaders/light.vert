#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec2 uvs;

void main() {
    gl_Position = projection * view * model * vec4(inPos, 1.0f);
    fragPos = inPos;
    uvs = inUv;
}
