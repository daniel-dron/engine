#version 430 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 fragPos;
    vec4 spacePos;
    vec4 fragPosLightSpace;
    mat3 TBN;
} fs_in;

layout (std140, binding = 0) uniform Matrices {
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};

layout(binding = 0) uniform sampler2D albedo_map;
layout(binding = 1) uniform sampler2D normal_map;
layout(binding = 2) uniform sampler2D mra_map;
layout(binding = 3) uniform sampler2D emissive_map;

uniform float metallic_factor = 1.0f;
uniform float roughness_factor = 1.0f;
uniform float emissive_factor = 1.0f;
uniform float ao_factor = 1.0f;

// lights
uniform vec3 lightPositions[4] = {
    vec3(7.7f, 2.0f, 10.5f), vec3(7.7f, 5.0f, 10.5f),
    vec3(-5.3f, 3.5f, 10.5f), vec3(4.0f, -1.0f, -2.5f)
};

uniform vec3 lightColors[4] = {
	vec3(50.0f, 50.0f, 50.0f), vec3(50.0f, 50.0f, 50.0f),
	vec3(50.0f, 50.0f, 50.0f), vec3(50.0f, 50.0f, 50.0f)        
};

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

vec3 getNormalFromMap()
{
    vec3 n = texture(normal_map, fs_in.uvs).rgb;
    n = n * 2.0f - 1.0f;
    return normalize(fs_in.TBN * n);

}

void main() {
    vec3 N = normalize(fs_in.normal);
    vec3 V = normalize(eyePos - fs_in.fragPos);

    // apply gamma correction to albedo
    vec3 albedo = texture(albedo_map, fs_in.uvs).rgb;
    vec3 emissive = texture(emissive_map, fs_in.uvs).rgb * emissive_factor;
    vec3 normal = getNormalFromMap();
    float metallic = texture(mra_map, fs_in.uvs).b * metallic_factor;
    float roughness = texture(mra_map, fs_in.uvs).g * roughness_factor;
    float ao = texture(mra_map, fs_in.uvs).r * ao_factor;

    vec3 Lo = vec3(0.0f);
    for(int i = 0; i < 4; ++i) {
        vec3 L = normalize(lightPositions[i] - fs_in.fragPos);
        vec3 H = normalize(V + L);

        float distance = length(lightPositions[i] - fs_in.fragPos);
        float attenuation = 1.0f / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // interpolate surface reflection between 0.04 (minimum) and the albedo value
        // in relation to the metallic factor of the material
        vec3 F0 = vec3(0.04); 
        F0      = mix(F0, albedo, metallic);
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

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo + emissive;

    // HDR tonemapping and gamma correction
    //color = color / (color + vec3(1.0f));
    //color = pow(color, vec3(1.0f/2.2f));

    // TODO: remove lol
    FragColor = vec4(color, 1.0f);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0f)
        BrightColor = vec4(FragColor.rgb, 1.0f);
    else
        BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}
