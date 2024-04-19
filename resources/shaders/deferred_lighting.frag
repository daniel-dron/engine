#version 430 core

out vec4 out_color; 
in vec2 tex_coords;

layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
    vec3 eye_pos;
};

layout(binding = 0) uniform sampler2D albedo_map;
layout(binding = 1) uniform sampler2D normal_map;
layout(binding = 2) uniform sampler2D mra_map;
layout(binding = 3) uniform sampler2D emissive_map;
layout(binding = 4) uniform sampler2D world_map;
layout(binding = 5) uniform samplerCube irradiance_map;
layout(binding = 6) uniform samplerCube prefilter_map;
layout(binding = 7) uniform sampler2D brdf_lut;
layout(binding = 8) uniform sampler2D shadow_map;

uniform mat4 light_space_matrix;

// lights
uniform vec3 lightPositions[4] = {
    vec3(7.7f, 2.0f, 10.5f), vec3(7.7f, 5.0f, 10.5f),
    vec3(-5.3f, 3.5f, 10.5f), vec3(4.0f, -1.0f, -2.5f)
};

uniform vec3 lightColors[4] = {
	vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
	vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)        
};

vec3 sun_position = { 1.0f, 1.0f, 0.0f };
vec3 sun_color = { 0.8f, 0.7f, 0.8f };
float sun_intensity = 10.0f;

float PI = 3.14159265359f;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

vec3 pbr(vec3 albedo, vec3 emissive, float metallic, float roughness, float ao, vec3 normal, vec3 view_dir, vec3 position) {
    vec3 N = normal;
    vec3 V = view_dir;

	// interpolate surface reflection between 0.04 (minimum) and the albedo value
	// in relation to the metallic factor of the material
	vec3 F0 = vec3(0.04); 
	F0      = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0f);
    for(int i = 0; i < 2; ++i) {
        vec3 L = normalize(lightPositions[i] - position);
        vec3 H = normalize(V + L);

        float distance = length(lightPositions[i] - position);
        float attenuation = 1.0f / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

		float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);

        vec3 numerator    = NDF * G * F;
        // add 0.0001 to the denominator to prevent divide by zero
        float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;

        kD *= 1.0f - metallic;

        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    //
    // sun
    //
    {
        vec3 L = normalize(sun_position - vec3(0.0f, 0.0f, 0.0f));
        vec3 H = normalize(V + L);

        vec3 radiance = sun_color * sun_intensity;

        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

		float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);

        vec3 numerator    = NDF * G * F;
        // add 0.0001 to the denominator to prevent divide by zero
        float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;

        kD *= 1.0f - metallic;

        float NdotL = max(dot(N, L), 0.0f);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // shadow
    vec4 pos_light_space = light_space_matrix * vec4(position, 1.0f);
    vec3 proj_coords = pos_light_space.xyz / pos_light_space.w;
    proj_coords = proj_coords * 0.5f + 0.5f;
    float closest_depth = texture(shadow_map, proj_coords.xy).r;
    float current_depth = proj_coords.z;
    float bias = max(0.005 * (1.0 - dot(normal, normalize(sun_position))), 0.005);  
    float shadow = current_depth - bias > closest_depth ? 1.0 : 0.0; 
    // if pixel is further than the shadow map, lets make it not shadow
    if (proj_coords.z > 1.0f)
        shadow = 0.0f;


    //vec3 kS = fresnelSchlick(max(dot(N, V), 0.0f), F0);
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec2 temp = vec2(max(dot(N, V), 0.0f), roughness);

    vec2 env_brdf = texture(brdf_lut, temp).rg;
    const float MAX_REFLECTION_LOD = 4.0;

    // specular
    vec3 R = reflect(-V, N); 
	vec3 prefilteredColor = textureLod(prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb; 
    vec3 specular = prefilteredColor * (F * env_brdf.x + env_brdf.y);

    vec3 kS = F;
    vec3 kD = 1.0f - kS;
    kD *= 1.0f - metallic;
    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 ambient =(kD * diffuse + specular) * ao;
    vec3 color = ambient +  (1.0f - shadow) *  (Lo + emissive);


    return color;
}

void main() {
	vec3 position = texture(world_map, tex_coords).rgb;

	vec3 normal = texture(normal_map, tex_coords).rgb;
	vec3 view_dir = normalize(eye_pos - position);

	vec3 albedo = texture(albedo_map, tex_coords).rgb;
	vec3 emissive = texture(emissive_map, tex_coords).rgb;
	float metallic = texture(mra_map, tex_coords).b;
	float roughness = texture(mra_map, tex_coords).g;
	float ao = texture(mra_map, tex_coords).r;

    vec3 pbr_color = pbr(albedo, emissive, metallic, roughness, ao, normal, view_dir, position);

    out_color = vec4(pbr_color, 1.0f);
}
