#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

layout (std140, binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model = mat4(1.0f);

out vec2 uvs;

void main() {
    uvs = uv;
    gl_Position = projection * view * model * vec4(position, 1.0f);
}