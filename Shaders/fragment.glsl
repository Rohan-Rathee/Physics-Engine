#version 460 core
out vec4 FragColor;

in vec4 FragPosLightSpace;
in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D texture1;
uniform sampler2D shadowMap;

float calculateShadow(vec4 fragPosLightSpace){
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0)
        return 0.0;
    if(projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = 0.001;
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}


void main()
{
    float shadow = calculateShadow(FragPosLightSpace);
    
    // Use passed color
    vec3 color = texture(texture1, TexCoord).rgb;
    
    // Basic lighting from light direction
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 normal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    float diff = max(dot(normal, lightDir), 0.3);
    
    // Apply shadow
    vec3 lighting = (1.0 - shadow) * diff * color;  
    FragColor = vec4(lighting, 1.0);
}
