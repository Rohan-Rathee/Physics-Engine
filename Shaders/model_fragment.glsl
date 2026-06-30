#version 460 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;
in mat3 TBN;


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metallicRoughness1;
uniform sampler2D texture_ao1;
uniform sampler2D texture_emissive1;


uniform bool  hasEmissiveMap;
uniform vec3  materialEmissive;
uniform float emissiveStrength;


uniform sampler2D   shadowMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;


uniform vec3  cameraPos;
uniform vec3  fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform bool  hasIBL;


uniform vec3  materialBaseColor;
uniform float materialMetallic;
uniform float materialRoughness;
uniform float materialAO;
uniform bool  hasNormalMap;
uniform bool  hasMetallicRoughnessMap;
uniform bool  hasAOMap;


#define MAX_LIGHTS 16

struct LightData {
    int   type;
    vec3  color;
    vec3  position;
    vec3  direction;
    float constant;
    float linear;
    float quadratic;
    float innerCutoff;
    float outerCutoff;
};

uniform int       u_numLights;
uniform LightData u_lights[MAX_LIGHTS];

const float PI = 3.14159265359;




float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0)
              * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness)
         * GeometrySchlickGGX(NdotL, roughness);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDirection)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 ||
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, -lightDirection)), 0.0005);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    float shadow = 0.0;

    for (int x = -2; x <= 2; ++x)
        for (int y = -2; y <= 2; ++y)
        {
            float pcfDepth = texture(shadowMap,
                                     projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }

    return shadow / 25.0;
}


vec3 ACESFilm(vec3 x)
{
    float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}




void main()
{

    vec4 albedoColor = texture(texture_diffuse1, TexCoord);
    if (albedoColor.a < 0.1)
        discard;





    vec3 albedo = albedoColor.rgb * materialBaseColor;


    vec3 N;
    if (hasNormalMap)
    {
        vec3 n = texture(texture_normal1, TexCoord).rgb;
        n = normalize(n * 2.0 - 1.0);
        N = normalize(TBN * n);
    }
    else
    {
        N = normalize(Normal);
    }


    float metallic  = materialMetallic;
    float roughness = materialRoughness;
    if (hasMetallicRoughnessMap)
    {



        vec3 mr = texture(texture_metallicRoughness1, TexCoord).rgb;
        roughness *= mr.g;
        metallic  *= mr.b;
    }
    roughness = clamp(roughness, 0.045, 1.0);


    float ao = hasAOMap ? texture(texture_ao1, TexCoord).r : materialAO;


    vec3 V  = normalize(cameraPos - FragPos);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);


    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_numLights; ++i)
    {
        LightData light = u_lights[i];

        vec3  L;
        float attenuation = 1.0;

        if (light.type == 1)
        {

            L = -light.direction;
        }
        else
        {
            vec3  toLight = light.position - FragPos;
            float dist    = length(toLight);
            L = toLight / dist;

            float denom  = light.constant
                         + light.linear    * dist
                         + light.quadratic * dist * dist;
            attenuation  = 1.0 / max(denom, 0.0001);

            if (light.type == 2)
            {

                float theta   = dot(L, -light.direction);
                float epsilon = light.innerCutoff - light.outerCutoff;
                attenuation  *= clamp((theta - light.outerCutoff) / epsilon,
                                      0.0, 1.0);
            }
        }

        float NdotL = max(dot(N, L), 0.0);

        vec3  H  = normalize(V + L);
        float D  = DistributionGGX(N, H, roughness);
        float G  = GeometrySmith(N, V, L, roughness);
        vec3  F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kD     = (vec3(1.0) - F) * (1.0 - metallic);
        float denom = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        vec3 spec   = (D * G * F) / denom;

        Lo += (kD * albedo / PI + spec) * light.color * attenuation * NdotL;
    }


    if (u_numLights > 0)
    {
        float shadow = calculateShadow(FragPosLightSpace, N,
                                       u_lights[0].direction);
        Lo *= (1.0 - shadow * 0.7);
    }


    vec3 ambient = vec3(0.0);
    if (hasIBL)
    {
        float NdotV = max(dot(N, V), 0.0);
        vec3 F  = fresnelSchlickRoughness(NdotV, F0, roughness);
        vec3 kD = (1.0 - F) * (1.0 - metallic);

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse    = irradiance * albedo;

        vec3  R = reflect(-V, N);
        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R,
                                           roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf     = texture(brdfLUT, vec2(NdotV, roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        ambient = (kD * diffuse + specular) * ao;
    }
    else
    {
        ambient = albedo * 0.3 * ao;
    }










    vec3 emissiveTex = hasEmissiveMap
        ? texture(texture_emissive1, TexCoord).rgb
        : vec3(1.0);

    vec3 emissive = emissiveTex * materialEmissive * emissiveStrength;


    vec3 result = Lo + ambient + emissive;


    float distToCamera = length(cameraPos - FragPos);
    float fogFactor    = clamp((fogEnd - distToCamera) / (fogEnd - fogStart),
                               0.0, 1.0);
    result = mix(fogColor, result, fogFactor);


    FragColor = vec4(result, albedoColor.a);

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    float knee   = 0.1;
    float rq     = clamp(brightness - 1.0 + knee, 0.0, 2.0 * knee);
    float weight = clamp((rq * rq) / (4.0 * knee + 0.00001), 0.0, 1.0);
    BrightColor  = vec4(result * max(weight, step(1.0, brightness)), 1.0);
}
