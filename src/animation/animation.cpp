#include "animation.h"
#include "assimp_glm_helpers.h"



#include "../utils/model_loader.h"

#include <cassert>
#include <iostream>
Animation::Animation(const aiScene* scene, Model* model, unsigned int animationIndex)
{
    assert(scene && scene->mRootNode);

    globalInverseTransform =
        glm::inverse(
            AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                scene->mRootNode->mTransformation
            )
        );

    if (!scene->HasAnimations() || animationIndex >= scene->mNumAnimations)
    {
        std::cerr << "Animation::Animation - requested animation index "
                  << animationIndex << " is not available (scene has "
                  << scene->mNumAnimations << " animations)" << std::endl;

        duration = 0.0f;
        ticksPerSecond = 25.0f;

        ReadHierarchyData(rootNode, scene->mRootNode);
        return;
    }

    aiAnimation* animation = scene->mAnimations[animationIndex];

    name = animation->mName.C_Str();
    duration = static_cast<float>(animation->mDuration);

    ticksPerSecond =
        static_cast<float>(animation->mTicksPerSecond);

    if (ticksPerSecond == 0.0f)
        ticksPerSecond = 25.0f;

    ReadHierarchyData(rootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
}

Bone* Animation::FindBone(const std::string& name)
{
    for (auto& bone : bones)
    {
        if (bone.GetBoneName() == name)
            return &bone;
    }
    return nullptr;
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{


    boneInfoMap = model.getBoneInfoMap();
    int boneCount = model.getBoneCount();

    int numChannels = static_cast<int>(animation->mNumChannels);
    bones.reserve(numChannels);

    for (int i = 0; i < numChannels; i++)
    {
        const aiNodeAnim* channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.C_Str();



        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo info;
            info.id = boneCount++;
            info.offset = glm::mat4(1.0f);
            boneInfoMap[boneName] = info;
        }

        bones.emplace_back(boneName, boneInfoMap[boneName].id, channel);
    }
}

void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);

    dest.name = src->mName.C_Str();
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = static_cast<int>(src->mNumChildren);

    for (unsigned int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
