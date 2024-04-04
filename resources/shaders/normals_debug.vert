#version 430 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;

out VS_OUT {
    vec3 normal;
} vs_out;

layout (std140, binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};
uniform mat4 model;

void main() {
    gl_Position = view * model * vec4(pos, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * normal, 0.0)));
}
