#version 330 core

out vec4 FragColor;

in vec3 normal;
in vec2 uvs;

uniform sampler2D aTexture;

void main() {
    vec4 color = texture(aTexture, uvs);
    FragColor = color;
}
