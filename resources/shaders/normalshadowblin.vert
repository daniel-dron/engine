#version 430 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tanget;
layout (location = 4) in vec3 bitanget;

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
    mat3 TBN;
} vs_out;

void main() {
    vs_out.normal = mat3(transpose(inverse(model))) * normal;
    vs_out.uvs = texCoord;
    vs_out.fragPos = vec3(model * vec4(position, 1.0));
    vs_out.spacePos = projection * view * model * vec4(position, 1.0);
    vs_out.fragPosLightSpace = lightSpaceMatrix * vec4(vs_out.fragPos, 1.0);

    vec3 T = normalize(vec3(model * vec4(tanget, 0.0)));
    vec3 B = normalize(vec3(model * vec4(bitanget, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal, 0.0)));
    vs_out.TBN = mat3(T, B, N);

    gl_Position = vs_out.spacePos;
}
