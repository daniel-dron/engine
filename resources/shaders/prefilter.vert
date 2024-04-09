#version 430 core

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model = mat4(1.0f);

out vec3 local_pos;

void main() {
    local_pos = position;
    gl_Position = projection * view * model * vec4(position, 1.0f);
}