#version 330 core 
out vec4 FragColor; 
in vec3 localPos; 
uniform samplerCube environmentMap; 
uniform float exposure;     
void main() 
{ 
    vec3 envColor = texture(environmentMap, localPos).rgb; 
      
    envColor = vec3(1.0) - exp(-envColor * exposure); 
      
    envColor = pow(envColor, vec3(1.0 / 2.2)); 
    FragColor = vec4(envColor, 1.0); 
} 
