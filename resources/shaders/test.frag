#version 430 core

out vec4 out_color;

layout(binding = 0) uniform sampler2D equirectangularMap;

in vec3 local_pos;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

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
    vec2 uv = SampleSphericalMap(normalize(local_pos)); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;

    out_color = vec4(aces(color), 1.0);
}