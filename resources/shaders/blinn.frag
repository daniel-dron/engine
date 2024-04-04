#version 430 core

out vec4 FragColor;

in VS_OUT {
    vec3 normal;
    vec2 uvs;
    vec3 fragPos;
    vec4 spacePos;
} fs_in;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
};

struct DirLight {
    vec3 direction;
    float _1;
    vec3 ambient;
    float _2;
    vec3 diffuse;
    float _3;
    vec3 specular;
};

struct Attenuation {
    float constant;
    float linear;
    float quadratic;
};

struct PointLight {
    vec3 position;
    float _1;
    vec3 ambient;
    float _2;
    vec3 diffuse;
    float _3;
    vec3 specular;
    float _4;
    Attenuation att;
};

layout (std140, binding = 0) uniform Matrices
{
    mat4 view;
    mat4 projection;
    vec3 eyePos;
};

layout (std140, binding = 1) uniform PointLightUniform {
    PointLight pointLights[10];
};

layout (std140, binding = 2) uniform DirectionalLightUniform {
    DirLight dirLight;
};

uniform Material material;
uniform samplerCube cubemap;

vec3 calcPointLight(PointLight light, vec3 norm, vec3 dirToView) {
    vec3 lightDir = normalize(light.position - fs_in.fragPos);
    vec3 halfwayDir = normalize(lightDir + dirToView);

    // attenuation
    float distance = length(light.position - fs_in.fragPos);
    float attenuation = 1.0f / (light.att.constant + light.att.linear * distance + light.att.quadratic * (distance * distance));

    // ambient
    vec3 ambient = light.ambient * texture(material.diffuse, fs_in.uvs).rgb;

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, fs_in.uvs).rgb;

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(norm, halfwayDir), 0.0f), 16.0f);
    vec3 specular = spec * light.specular;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcDirectionalLight(DirLight light, vec3 norm, vec3 dirToView) {
    vec3 lightDir = normalize(-light.direction.xyz);
    vec3 halfwayDir = normalize(lightDir + dirToView);

    // ambient
    vec3 ambient = light.ambient.rgb * texture(material.diffuse, fs_in.uvs).rgb;

    // diffuse
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = light.diffuse.rgb * diff * texture(material.diffuse, fs_in.uvs).rgb;

    // specular
    vec3 reflectDir = reflect(-lightDir, norm);
       float spec = pow(max(dot(norm, halfwayDir), 0.0f), 16.0f);
    vec3 specular = spec * light.specular.rgb;

    return (ambient + diffuse + specular);
}

void main() {
    vec3 norm = normalize(fs_in.normal);
    vec3 dirToView = normalize(eyePos - fs_in.fragPos);

    vec3 pointLightColor = calcPointLight(pointLights[0], norm, dirToView);
    vec3 directionalLightColor = calcDirectionalLight(dirLight, norm, dirToView);

    vec3 result = pointLightColor + directionalLightColor;

    vec3 I = normalize(fs_in.fragPos - eyePos);
    vec3 R = refract(I, norm, 1.0f / 1.52f);

    FragColor = vec4(result, 1.0f);

    //gl_FragDepth = fs_in.spacePos.z / fs_in.spacePos.w;
}
