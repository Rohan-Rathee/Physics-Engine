#include "animator.h" 

#include <cmath> 

#include <algorithm> 

#include <glm/gtc/matrix_transform.hpp> 

#include <glm/gtc/quaternion.hpp> 

  

  

  

Animator::Animator() 

{ 

    finalBoneMatrices.resize(100, glm::mat4(1.0f)); 

} 

  

  

  

void Animator::PlayAnimation(Animation* animation) 

{ 

    currentAnimation = animation; 

    currentTime      = 0.0f; 

      

    BlendLayer layer; 

    layer.animation = animation; 

    layer.weight    = 1.0f; 

    layer.time      = 0.0f; 

    blendLayers     = { layer }; 

} 

void Animator::UpdateAnimation(float deltaTime) 

{ 

    if (blendLayers.empty()) 

        return; 

      

    for (auto& layer : blendLayers) 

    { 

        if (!layer.animation) 

            continue; 

        float duration = layer.animation->GetDuration(); 

        float tps      = layer.animation->GetTicksPerSecond(); 

        if (duration <= 0.0f || tps <= 0.0f) 

            continue; 

        layer.time += tps * deltaTime; 

        layer.time  = std::fmod(layer.time, duration); 

        if (layer.time < 0.0f) 

            layer.time += duration; 

    } 

      

      

    if (!blendLayers.empty() && blendLayers[0].animation) 

        currentTime = blendLayers[0].time; 

      

      

      

      

      

      

    BlendLayer* activeLayer = nullptr; 

    int activeCount = 0; 

    for (auto& layer : blendLayers) 

    { 

        if (layer.animation && layer.weight > 0.0f) 

        { 

            activeLayer = &layer; 

            activeCount++; 

            if (activeCount > 1) 

                break; 

        } 

    } 

    if (activeCount <= 1) 

    { 

        if (activeLayer) 

        { 

            CalculateBoneTransform( 

                &activeLayer->animation->GetRootNode(), 

                glm::mat4(1.0f), 

                activeLayer->animation, 

                activeLayer->time 

            ); 

        } 

          

          

    } 

    else 

    { 

        CalculateBlendedBoneTransforms(); 

    } 

} 

  

  

  

void Animator::SetBlendLayers(std::vector<BlendLayer> layers) 

{ 

    if (layers.empty()) 

        return; 

      

    if (layers[0].animation) 

    { 

        currentAnimation = layers[0].animation; 

        currentTime      = layers[0].time; 

    } 

    blendLayers = std::move(layers); 

} 

void Animator::SetLayerWeight(size_t layerIndex, float weight) 

{ 

    if (layerIndex < blendLayers.size()) 

        blendLayers[layerIndex].weight = weight; 

} 

  

  

  

void Animator::CalculateBoneTransform(const AssimpNodeData* node, 

                                      const glm::mat4&      parentTransform, 

                                      Animation*            anim, 

                                      float                 animTime) 

{ 

    const std::string& nodeName    = node->name; 

    glm::mat4          nodeTransform = node->transformation; 

    Bone* bone = anim->FindBone(nodeName); 

    if (bone) 

    { 

        bone->Update(animTime); 

        nodeTransform = bone->GetLocalTransform(); 

    } 

    glm::mat4 globalTransformation = parentTransform * nodeTransform; 

    const auto& boneInfoMap = anim->GetBoneIDMap(); 

    auto        it          = boneInfoMap.find(nodeName); 

    if (it != boneInfoMap.end()) 

    { 

        int               index  = it->second.id; 

        const glm::mat4&  offset = it->second.offset; 

        if (index >= 0 && index < static_cast<int>(finalBoneMatrices.size())) 

            finalBoneMatrices[index] = 

                anim->GetGlobalInverseTransform() * globalTransformation * offset; 

    } 

    for (int i = 0; i < node->childrenCount; i++) 

        CalculateBoneTransform(&node->children[i], globalTransformation, anim, animTime); 

} 

  

  

  

Animator::DecomposedTransform Animator::DecomposeMatrix(const glm::mat4& m) 

{ 

    DecomposedTransform out; 

    out.translation = glm::vec3(m[3]); 

    glm::vec3 col0(m[0]), col1(m[1]), col2(m[2]); 

    out.scale.x = glm::length(col0); 

    out.scale.y = glm::length(col1); 

    out.scale.z = glm::length(col2); 

    if (out.scale.x > 1e-6f) col0 /= out.scale.x; 

    if (out.scale.y > 1e-6f) col1 /= out.scale.y; 

    if (out.scale.z > 1e-6f) col2 /= out.scale.z; 

    glm::mat3 rotMat(col0, col1, col2); 

    out.rotation = glm::quat_cast(rotMat); 

    return out; 

} 

glm::mat4 Animator::RecomposeMatrix(const DecomposedTransform& t) 

{ 

    glm::mat4 m = glm::mat4_cast(glm::normalize(t.rotation)); 

    m[0] *= t.scale.x; 

    m[1] *= t.scale.y; 

    m[2] *= t.scale.z; 

    m[3]  = glm::vec4(t.translation, 1.0f); 

    return m; 

} 

  

  

void Animator::CollectBoneTransforms( 

    const AssimpNodeData*   node, 

    const glm::mat4&        parentTransform, 

    Animation*              anim, 

    float                   animTime, 

    std::vector<glm::mat4>& outGlobal) 

{ 

    const std::string& nodeName     = node->name; 

    glm::mat4          nodeTransform = node->transformation; 

    Bone* bone = anim->FindBone(nodeName); 

    if (bone) 

    { 

        bone->Update(animTime); 

        nodeTransform = bone->GetLocalTransform(); 

    } 

    glm::mat4 globalTransformation = parentTransform * nodeTransform; 

    const auto& boneInfoMap = anim->GetBoneIDMap(); 

    auto        it          = boneInfoMap.find(nodeName); 

    if (it != boneInfoMap.end()) 

    { 

        int index = it->second.id; 

        if (index >= 0 && index < static_cast<int>(outGlobal.size())) 

            outGlobal[index] = globalTransformation; 

    } 

    for (int i = 0; i < node->childrenCount; i++) 

        CollectBoneTransforms(&node->children[i], globalTransformation, anim, animTime, outGlobal); 

} 

  

  

void Animator::CalculateBlendedBoneTransforms() 

{ 

    const int boneCount = static_cast<int>(finalBoneMatrices.size()); 

      

    float totalWeight = 0.0f; 

    for (const auto& layer : blendLayers) 

        if (layer.animation) 

            totalWeight += layer.weight; 

    if (totalWeight < 1e-6f) 

        return; 

      

      

      

    if (static_cast<int>(blendAccumScratch.size()) != boneCount) 

        blendAccumScratch.resize(boneCount); 

    for (auto& a : blendAccumScratch) 

        a.hasValue = false;   

    if (static_cast<int>(blendGlobalScratch.size()) != boneCount) 

        blendGlobalScratch.resize(boneCount); 

    for (const auto& layer : blendLayers) 

    { 

        if (!layer.animation || layer.weight <= 0.0f) 

            continue; 

        float normWeight = layer.weight / totalWeight; 

          

        std::fill(blendGlobalScratch.begin(), blendGlobalScratch.end(), glm::mat4(1.0f)); 

        CollectBoneTransforms( 

            &layer.animation->GetRootNode(), 

            glm::mat4(1.0f), 

            layer.animation, 

            layer.time, 

            blendGlobalScratch 

        ); 

          

        const auto& boneInfoMap = layer.animation->GetBoneIDMap(); 

        for (const auto& [boneName, boneInfo] : boneInfoMap) 

        { 

            int index = boneInfo.id; 

            if (index < 0 || index >= boneCount) 

                continue; 

            DecomposedTransform dt = DecomposeMatrix(blendGlobalScratch[index]); 

            BoneAccum& ba = blendAccumScratch[index]; 

            if (!ba.hasValue) 

            { 

                ba.translation = dt.translation * normWeight; 

                ba.rotation    = dt.rotation;            

                ba.scale       = dt.scale * normWeight; 

                ba.hasValue    = true; 

                ba.weightSoFar = normWeight; 

            } 

            else 

            { 

                ba.translation += dt.translation * normWeight; 

                ba.scale       += dt.scale * normWeight; 

                  

                  

                float blendFraction = normWeight / (ba.weightSoFar + normWeight); 

                ba.rotation    = glm::slerp(ba.rotation, dt.rotation, blendFraction); 

                ba.weightSoFar += normWeight; 

            } 

        } 

    } 

      

      

      

    Animation* refAnim = nullptr; 

    for (const auto& layer : blendLayers) 

        if (layer.animation) { refAnim = layer.animation; break; } 

    if (!refAnim) 

        return; 

    const glm::mat4&    globalInverse = refAnim->GetGlobalInverseTransform(); 

    const auto&         boneInfoMap   = refAnim->GetBoneIDMap(); 

    for (const auto& [boneName, boneInfo] : boneInfoMap) 

    { 

        int index = boneInfo.id; 

        if (index < 0 || index >= boneCount) 

            continue; 

        if (!blendAccumScratch[index].hasValue) 

            continue; 

        glm::mat4 blendedGlobal = RecomposeMatrix({ 

            blendAccumScratch[index].translation, 

            blendAccumScratch[index].rotation, 

            blendAccumScratch[index].scale 

        }); 

        finalBoneMatrices[index] = 

            globalInverse * blendedGlobal * boneInfo.offset; 

    } 

}