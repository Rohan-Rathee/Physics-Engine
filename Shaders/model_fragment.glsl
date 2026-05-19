#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
    // Sample the diffuse texture
    vec4 diffuseColor = texture(texture_diffuse1, TexCoord);
    
    // If the texture lookup fails or is transparent, use a default color
    if (diffuseColor.a < 0.1) {
        discard;
    }
    
    // Simple lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(norm, lightDir), 0.2);
    
    vec3 result = diffuseColor.rgb * diff;
    FragColor = vec4(result, diffuseColor.a);
}
