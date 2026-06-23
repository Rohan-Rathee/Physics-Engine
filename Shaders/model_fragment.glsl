#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D shadowMap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

uniform bool hasIBL;

uniform vec3 cameraPos;
uniform vec3 lightDir;

uniform float ao;
uniform float metallic;
uniform float roughness;




float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.z > 1.0 ||
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = 0.0008;

    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float shadow = 0.0;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth =
                texture(
                    shadowMap,
                    projCoords.xy + vec2(x, y) * texelSize
                ).r;

            shadow += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}



void main()
{
    vec4 diffuseColor = texture(texture_diffuse1, TexCoord);

    if (diffuseColor.a < 0.1)
        discard;

    vec3 albedo = diffuseColor.rgb;

    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);


    vec3 ambient =
        albedo * 0.1 ;  


    float NdotL =
        max(dot(N, L), 0.0);

    vec3 diffuse =
        albedo * NdotL;


    float shadow =
        calculateShadow(
            FragPosLightSpace,
            N,
            lightDir
        );

    vec3 result =
        ambient +
        diffuse * (1.0 - shadow);


    float distanceToCamera =
        length(cameraPos - FragPos);

    float fogFactor =
        (fogEnd - distanceToCamera) /
        (fogEnd - fogStart);

    fogFactor =
        clamp(
            fogFactor,
            0.0,
            1.0
        );

    result =
        mix(
            fogColor,
            result,
            fogFactor
        );

    FragColor =
        vec4(
            result,
            diffuseColor.a
        );
}