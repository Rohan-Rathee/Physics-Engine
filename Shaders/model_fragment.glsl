#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D shadowMap;
uniform vec3 lightDir;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || 
       projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    float bias = 0.001;

    
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}


void main()
{
    vec3 normal = normalize(Normal);
    vec4 diffuseColor = texture(texture_diffuse1, TexCoord);
    
    if (diffuseColor.a < 0.1) {
        discard;
    }
    
    float shadow = calculateShadow(FragPosLightSpace, normal, lightDir);
    float lighting = (1.0 - shadow);
    vec3 result = diffuseColor.rgb * max(lighting, 0.3);
    
    FragColor = vec4(result, diffuseColor.a);

}