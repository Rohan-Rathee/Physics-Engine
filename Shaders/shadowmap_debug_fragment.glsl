#version 460 core 

out vec4 FragColor; 

in vec2 TexCoord; 

uniform sampler2D shadowMap; 

void main() 

{ 

      

    float depthValue = texture(shadowMap, TexCoord).r; 

      

    vec3 col = vec3(depthValue); 

    FragColor = vec4(col, 1.0); 

} 

   