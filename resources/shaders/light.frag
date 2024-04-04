#version 330 core

out vec4 FragColor;

in vec3 fragPos;
in vec2 uvs;

uniform sampler2D text;

void main() {
    vec4 color = texture(text, uvs);
    FragColor = vec4(color);
}
