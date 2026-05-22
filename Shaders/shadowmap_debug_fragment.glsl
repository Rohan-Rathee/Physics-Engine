#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D shadowMap;

void main()
{
    // Sample the depth value from the shadow map
    float depthValue = texture(shadowMap, TexCoord).r;
    // Visualize the depth map (linearize for better visualization)
    vec3 col = vec3(depthValue);
    FragColor = vec4(col, 1.0);
}
