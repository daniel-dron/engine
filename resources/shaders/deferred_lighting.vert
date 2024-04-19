#version 430 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoords;

out vec2 tex_coords;

void main() {
	tex_coords = texcoords;
	gl_Position = vec4(position, 0.0f, 1.0f);
}