#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float gamma;

void main()
{ 
    vec3 color = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(pow(color, vec3(1.0f/gamma)), 1.0f);
}
