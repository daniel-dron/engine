#version 430 core
out vec4 FragColor;
  
in vec2 TexCoords;

layout(binding = 0) uniform sampler2D screenTexture;
layout(binding = 1) uniform sampler2D bloomBlur;

uniform float exposure;

void main()
{ 

    vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    hdrColor += bloomColor;

    // tone mapping
    hdrColor = hdrColor / (hdrColor + vec3(1.0));

    // gamma correction
    hdrColor = pow(hdrColor, vec3(1.0 / 2.2));

    FragColor = vec4(hdrColor, 1.0f);
}
