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
    float bias = 0.004;
    //variables:
    //1) bias: base bias
    //2) bias scaling factor: 10.0 is an arbitrary value that determines how much the bias increases with angle. You can adjust this based on your scene's needs.
    //3) minimum bias: 0.0005 is a small value to prevent the bias from becoming too small when the angle is close to 90 degrees. You can adjust this as well.
    //the bias is calculated based on the angle between the normal and the light direction. When the angle is small (light is almost perpendicular to the surface), the bias is small. As the angle increases (light grazes the surface), the bias increases to help prevent shadow acne. The max function ensures that the bias never goes below a certain minimum value, which helps maintain shadow quality even at steep angles.

    
    //texel size bias
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    return shadow / 9.0;
}


void main()
{
    vec3 normal = normalize(Normal);
    vec4 diffuseColor = texture(texture_diffuse1, TexCoord);
    vec3 lightDirection = normalize(-lightDir);

    
    if (diffuseColor.a < 0.1) {
        discard;
    }

    float shadow = calculateShadow(FragPosLightSpace, Normal, lightDir);
    float ambient = 0.001;

    float diffuse = max(dot(normal, lightDirection), 0.0);
    diffuse = pow(diffuse, 0.85);
    //how to make it diffuse earlier
    float lighting = pow(ambient + diffuse * (1.0 - shadow), 0.3);
    //lighting = pow(ambient + (1.0 - shadow), 1);

    vec3 result = diffuseColor.rgb * lighting;
    
    FragColor = vec4(result, diffuseColor.a);
    
    //FragColor = vec4(vec3(shadow), 1.0);
    //normal visualization
    //FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    //normals facing camera visualization

    //SHADOW MAP SHOWING
    //shadow map real visualization
    //FragColor = vec4(vec3(shadow), 1.0);
    //only shadowed areas

}