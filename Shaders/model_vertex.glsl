#version 460 core 
layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoord; 
layout (location = 3) in ivec4 aBoneIDs; 
layout (location = 4) in vec4 aWeights; 
const int MAX_BONES = 100; 
const int MAX_BONE_INFLUENCE = 4; 
uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 
uniform mat4 lightSpaceMatrix; 
uniform mat4 finalBonesMatrices[MAX_BONES]; 
uniform bool hasAnimation; 
out vec2 TexCoord; 
out vec3 Normal; 
out vec3 FragPos; 
out vec4 FragPosLightSpace; 
void main() 
{ 
    vec4 totalPosition = vec4(0.0); 
    vec3 totalNormal = vec3(0.0); 
    if (hasAnimation) 
    { 
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) 
        { 
            int boneID = aBoneIDs[i]; 
            float weight = aWeights[i]; 
            if (boneID == -1) 
                continue; 
            if (boneID >= MAX_BONES) 
            { 
                totalPosition = vec4(aPos, 1.0); 
                totalNormal = aNormal; 
                break; 
            } 
            vec4 skinnedPosition = finalBonesMatrices[boneID] * vec4(aPos, 1.0); 
            totalPosition += skinnedPosition * weight; 
            vec3 skinnedNormal = mat3(finalBonesMatrices[boneID]) * aNormal; 
            totalNormal += skinnedNormal * weight; 
        } 
          
          
        if (totalPosition.w == 0.0) 
        { 
            totalPosition = vec4(aPos, 1.0); 
            totalNormal = aNormal; 
        } 
    } 
    else 
    { 
        totalPosition = vec4(aPos, 1.0); 
        totalNormal = aNormal; 
    } 
    vec4 worldPos = model * totalPosition; 
    FragPosLightSpace = lightSpaceMatrix * worldPos; 
    FragPos = vec3(worldPos); 
    Normal = mat3(transpose(inverse(model))) * totalNormal; 
    TexCoord = aTexCoord; 
    gl_Position = projection * view * worldPos; 
} 
   