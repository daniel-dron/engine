#version 330 core
out vec4 FragColor;

in vec3 texUvs;

uniform samplerCube skybox;

vec3 aces(vec3 color) {
	color *= 0.6;
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0,
		   1.0);
}

void main() {
    FragColor = vec4(aces(texture(skybox, texUvs).rgb), 1.0f);
}
