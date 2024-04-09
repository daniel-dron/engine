#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D bloomBlur;

uniform float exposure;

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

void main()
{ 

    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    //hdrColor += bloomColor;

    // tone mapping
	aces(hdrColor);

    // gamma correction
    hdrColor = pow(hdrColor, vec3(1.0 / 2.2));

    FragColor = vec4(hdrColor, 1.0f);
}
