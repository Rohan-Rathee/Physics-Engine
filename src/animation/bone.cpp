#include "bone.h"
#include "assimp_glm_helpers.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    : name(name), id(ID)
{
    numPositions = channel->mNumPositionKeys;
    for (int i = 0; i < numPositions; i++)
    {
        KeyPosition data;
        data.position = AssimpGLMHelpers::GetGLMVec(channel->mPositionKeys[i].mValue);
        data.timeStamp = static_cast<float>(channel->mPositionKeys[i].mTime);
        positions.push_back(data);
    }

    numRotations = channel->mNumRotationKeys;
    for (int i = 0; i < numRotations; i++)
    {
        KeyRotation data;
        data.orientation = AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[i].mValue);
        data.timeStamp = static_cast<float>(channel->mRotationKeys[i].mTime);
        rotations.push_back(data);
    }

    numScalings = channel->mNumScalingKeys;
    for (int i = 0; i < numScalings; i++)
    {
        KeyScale data;
        data.scale = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[i].mValue);
        data.timeStamp = static_cast<float>(channel->mScalingKeys[i].mTime);
        scales.push_back(data);
    }
}

void Bone::Update(float animationTime)
{
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);
    localTransform = translation * rotation * scale;
}

int Bone::GetPositionIndex(float animationTime) const
{
    for (int i = 0; i < numPositions - 1; i++)
    {
        if (animationTime < positions[i + 1].timeStamp)
            return i;
    }
    return numPositions > 1 ? numPositions - 2 : 0;
}

int Bone::GetRotationIndex(float animationTime) const
{
    for (int i = 0; i < numRotations - 1; i++)
    {
        if (animationTime < rotations[i + 1].timeStamp)
            return i;
    }
    return numRotations > 1 ? numRotations - 2 : 0;
}

int Bone::GetScaleIndex(float animationTime) const
{
    for (int i = 0; i < numScalings - 1; i++)
    {
        if (animationTime < scales[i + 1].timeStamp)
            return i;
    }
    return numScalings > 1 ? numScalings - 2 : 0;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const
{
    float framesDiff = nextTimeStamp - lastTimeStamp;
    if (framesDiff <= 0.0f)
        return 0.0f;

    float midWayLength = animationTime - lastTimeStamp;
    float scaleFactor = midWayLength / framesDiff;
    return glm::clamp(scaleFactor, 0.0f, 1.0f);
}

glm::mat4 Bone::InterpolatePosition(float animationTime) const
{
    if (numPositions == 0)
        return glm::mat4(1.0f);
    if (numPositions == 1)
        return glm::translate(glm::mat4(1.0f), positions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);

    glm::vec3 finalPosition = glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime) const
{
    if (numRotations == 0)
        return glm::mat4(1.0f);
    if (numRotations == 1)
        return glm::mat4_cast(glm::normalize(rotations[0].orientation));

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);

    glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);
    return glm::mat4_cast(glm::normalize(finalRotation));
}

glm::mat4 Bone::InterpolateScaling(float animationTime) const
{
    if (numScalings == 0)
        return glm::mat4(1.0f);
    if (numScalings == 1)
        return glm::scale(glm::mat4(1.0f), scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(scales[p0Index].timeStamp, scales[p1Index].timeStamp, animationTime);

    glm::vec3 finalScale = glm::mix(scales[p0Index].scale, scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}
