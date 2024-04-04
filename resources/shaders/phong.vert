#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;

layout (std140, binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};

out VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 fragPos;
    vec4 spacePos;
} vs_out;

void main() {
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.uvs = aTexCoords;
    vs_out.fragPos = vec3(model * vec4(aPos, 1.0f));
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    vs_out.spacePos = gl_Position;
}
