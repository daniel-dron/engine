#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 model;
layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};

uniform mat4 lightSpaceMatrix;

out VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 fragPos;
    vec4 spacePos;
    vec4 fragPosLightSpace;
} vs_out;

void main() {
    vs_out.normal = mat3(transpose(inverse(model))) * normal;
    vs_out.uvs = texCoord;
    vs_out.fragPos = vec3(model * vec4(position, 1.0));
    vs_out.spacePos = projection * view * model * vec4(position, 1.0);
    vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);
    gl_Position = vs_out.spacePos;
}
