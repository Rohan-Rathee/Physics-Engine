#pragma once 
#include <vector> 
#include <string> 
#include <glm/glm.hpp> 
#include <glm/gtc/quaternion.hpp> 
#include "animation.h" 

struct BlendLayer 
{ 
    Animation* animation = nullptr; 
    float      weight    = 1.0f; 
    float      time      = 0.0f;     
}; 

class Animator 
{ 
public: 
    Animator(); 
    void PlayAnimation(Animation* animation); 
    void UpdateAnimation(float deltaTime); 
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const { return finalBoneMatrices; } 
    void SetBlendLayers(std::vector<BlendLayer> layers); 
    void SetLayerWeight(size_t layerIndex, float weight); 
    size_t GetLayerCount() const { return blendLayers.size(); } 
    
private: 
    struct DecomposedTransform 
    { 
        glm::vec3 translation{ 0.f }; 
        glm::quat rotation{ 1.f, 0.f, 0.f, 0.f }; 
        glm::vec3 scale{ 1.f }; 
    }; 
    static DecomposedTransform DecomposeMatrix(const glm::mat4& m); 
    static glm::mat4            RecomposeMatrix(const DecomposedTransform& t); 
    void CollectBoneTransforms( 
        const AssimpNodeData*              node, 
        const glm::mat4&                   parentTransform, 
        Animation*                         anim, 
        float                              animTime, 
        std::vector<glm::mat4>&            outGlobal     
    ); 
    void CalculateBlendedBoneTransforms(); 
    void CalculateBoneTransform(const AssimpNodeData* node, 
                                const glm::mat4&      parentTransform, 
                                Animation*            anim, 
                                float                 animTime); 
    std::vector<BlendLayer>  blendLayers; 
    std::vector<glm::mat4>   finalBoneMatrices; 
    struct BoneAccum 
    { 
        glm::vec3 translation{ 0.f }; 
        glm::quat rotation{ 1.f, 0.f, 0.f, 0.f }; 
        glm::vec3 scale{ 0.f }; 
        bool      hasValue{ false }; 
        float     weightSoFar{ 0.f }; 
    }; 
    std::vector<BoneAccum> blendAccumScratch; 
    std::vector<glm::mat4> blendGlobalScratch; 
    Animation* currentAnimation = nullptr; 
    float      currentTime      = 0.0f; 
}; 