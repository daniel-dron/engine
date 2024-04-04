#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

vec2 quadPosition = vec2(0.0, 0.0);
vec2 quadSize = vec2(1.0, 1.0);

void main()
{
    vec2 transformedPosition = aPos.xy * quadSize + quadPosition;
    gl_Position = vec4(transformedPosition, 0.0, 1.0);
    TexCoords = aTexCoords;
}  
