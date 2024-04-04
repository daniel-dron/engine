#version 430 core

layout (location = 0) in vec3 pos;

out vec3 texUvs;

layout (std140, binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};

void main() {
    // remove translation units from view matrix
    mat4 view = mat4(mat3(view));

    texUvs = pos;
    vec4 pos = projection * view * vec4(pos, 1.0);
    gl_Position = pos.xyww;
}
