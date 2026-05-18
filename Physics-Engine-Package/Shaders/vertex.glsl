#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 instanceMatrix; 


uniform mat4 view;
uniform mat4 projection;

out vec3 vFragPos;
out vec2 TexCoord;


void main()
{   
    vec4 worldPos = instanceMatrix * vec4(aPos, 1.0);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    vFragPos = vec3(view * worldPos);
    gl_Position = projection * view * worldPos;
}