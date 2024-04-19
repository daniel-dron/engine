#version 430 core

layout (location = 0) out vec4 g_albedo;
layout (location = 1) out vec4 g_normals;
layout (location = 2) out vec4 g_mra;
layout (location = 3) out vec4 g_emissive;
layout (location = 4) out vec4 g_world;

in VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 frag_pos;
    mat3 tbn;
} fs_in;

layout (std140, binding = 0) uniform Matrices {
	mat4 view;
	mat4 projection;
	vec3 eye_pos;
};

layout(binding = 0) uniform sampler2D albedo_map;
layout(binding = 1) uniform sampler2D normal_map;
layout(binding = 2) uniform sampler2D mra_map;
layout(binding = 3) uniform sampler2D emissive_map;

uniform float metallic_factor = 1.0f;
uniform float roughness_factor = 1.0f;
uniform float emissive_factor = 1.0f;
uniform float ao_factor = 1.0f;

void main() {
    // albedo
    g_albedo = texture(albedo_map, fs_in.uvs);

    //normals
    vec3 n = texture(normal_map, fs_in.uvs).rgb;
    n = n * 2.0f - 1.0f;
    g_normals = vec4(normalize(fs_in.tbn * n), 0.0f);

    // emissive
    g_emissive = texture(emissive_map, fs_in.uvs) * emissive_factor;

    // mra
    float m = texture(mra_map, fs_in.uvs).b * metallic_factor;
    float r = texture(mra_map, fs_in.uvs).g * roughness_factor;
    float a = texture(mra_map, fs_in.uvs).r * ao_factor;
    g_mra = vec4(a, r, m, 0.0f);

    // world
    g_world = vec4(fs_in.frag_pos, 1.0f);
}
