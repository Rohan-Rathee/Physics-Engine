#include "animator.h"
#include <cmath>

Animator::Animator()
{
    finalBoneMatrices.resize(100, glm::mat4(1.0f));
}

void Animator::PlayAnimation(Animation* animation)
{
    currentAnimation = animation;
    currentTime = 0.0f;
}

void Animator::UpdateAnimation(float deltaTime)
{
    if (!currentAnimation)
        return;

    float duration = currentAnimation->GetDuration();
    if (duration <= 0.0f)
        return;

    currentTime += currentAnimation->GetTicksPerSecond() * deltaTime;
    currentTime = std::fmod(currentTime, duration);
    if (currentTime < 0.0f)
        currentTime += duration;

    CalculateBoneTransform(&currentAnimation->GetRootNode(), glm::mat4(1.0f));
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform)
{
    const std::string& nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = currentAnimation->FindBone(nodeName);
    if (bone)
    {
        bone->Update(currentTime);
        nodeTransform = bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    const auto& boneInfoMap = currentAnimation->GetBoneIDMap();
    auto it = boneInfoMap.find(nodeName);
    if (it != boneInfoMap.end())
    {
        int index = it->second.id;
        const glm::mat4& offset = it->second.offset;
        if (index >= 0 && index < static_cast<int>(finalBoneMatrices.size()))
            finalBoneMatrices[index] =
    currentAnimation->GetGlobalInverseTransform()
    * globalTransformation
    * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}
