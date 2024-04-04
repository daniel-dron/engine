#version 430 core

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};

out vec3 nearPoint;
out vec3 farPoint;

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    nearPoint = UnprojectPoint(aPos.x, aPos.y, 0.0f, view, projection).xyz;
    farPoint = UnprojectPoint(aPos.x, aPos.y, 1.0f, view, projection).xyz;
    gl_Position = vec4(aPos, 1.0);
}
