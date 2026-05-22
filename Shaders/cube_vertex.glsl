#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec4 FragPosLightSpace;
out vec3 FragPos;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPosLightSpace = lightSpaceMatrix * worldPos;
    FragPos = worldPos.xyz;
    gl_Position = projection * view * worldPos;
}
