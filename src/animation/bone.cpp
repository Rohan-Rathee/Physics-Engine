#include "bone.h" 
#include "assimp_glm_helpers.h" 
#include <glm/gtc/quaternion.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
  
Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel) 
    : name(name), id(ID) 
{ 
    numPositions = channel->mNumPositionKeys; 
    positions.reserve(numPositions); 
    for (int i = 0; i < numPositions; i++) 
    { 
        KeyPosition data; 
        data.position  = AssimpGLMHelpers::GetGLMVec(channel->mPositionKeys[i].mValue); 
        data.timeStamp = static_cast<float>(channel->mPositionKeys[i].mTime); 
        positions.push_back(data); 
    } 
    numRotations = channel->mNumRotationKeys; 
    rotations.reserve(numRotations); 
    for (int i = 0; i < numRotations; i++) 
    { 
        KeyRotation data; 
        data.orientation = AssimpGLMHelpers::GetGLMQuat(channel->mRotationKeys[i].mValue); 
        data.timeStamp   = static_cast<float>(channel->mRotationKeys[i].mTime); 
        rotations.push_back(data); 
    } 
    numScalings = channel->mNumScalingKeys; 
    scales.reserve(numScalings); 
    for (int i = 0; i < numScalings; i++) 
    { 
        KeyScale data; 
        data.scale     = AssimpGLMHelpers::GetGLMVec(channel->mScalingKeys[i].mValue); 
        data.timeStamp = static_cast<float>(channel->mScalingKeys[i].mTime); 
        scales.push_back(data); 
    } 
} 
  
void Bone::Update(float animationTime) 
{ 
    glm::mat4 translation = InterpolatePosition(animationTime); 
    glm::mat4 rotation    = InterpolateRotation(animationTime); 
    glm::mat4 scale       = InterpolateScaling(animationTime); 
    localTransform        = translation * rotation * scale; 
} 
  
  
  
  
  
  
  
  
int Bone::GetPositionIndex(float animationTime) const 
{ 
    if (numPositions <= 1) 
        return 0; 
      
    if (animationTime < positions[m_LastPosIdx].timeStamp) 
        m_LastPosIdx = 0; 
      
    while (m_LastPosIdx < numPositions - 2 && 
           animationTime >= positions[m_LastPosIdx + 1].timeStamp) 
        ++m_LastPosIdx; 
    return m_LastPosIdx; 
} 
int Bone::GetRotationIndex(float animationTime) const 
{ 
    if (numRotations <= 1) 
        return 0; 
    if (animationTime < rotations[m_LastRotIdx].timeStamp) 
        m_LastRotIdx = 0; 
    while (m_LastRotIdx < numRotations - 2 && 
           animationTime >= rotations[m_LastRotIdx + 1].timeStamp) 
        ++m_LastRotIdx; 
    return m_LastRotIdx; 
} 
int Bone::GetScaleIndex(float animationTime) const 
{ 
    if (numScalings <= 1) 
        return 0; 
    if (animationTime < scales[m_LastScaleIdx].timeStamp) 
        m_LastScaleIdx = 0; 
    while (m_LastScaleIdx < numScalings - 2 && 
           animationTime >= scales[m_LastScaleIdx + 1].timeStamp) 
        ++m_LastScaleIdx; 
    return m_LastScaleIdx; 
} 
  
float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const 
{ 
    float framesDiff = nextTimeStamp - lastTimeStamp; 
    if (framesDiff <= 0.0f) 
        return 0.0f; 
    float midWayLength = animationTime - lastTimeStamp; 
    return glm::clamp(midWayLength / framesDiff, 0.0f, 1.0f); 
} 
glm::mat4 Bone::InterpolatePosition(float animationTime) const 
{ 
    if (numPositions == 0) return glm::mat4(1.0f); 
    if (numPositions == 1) return glm::translate(glm::mat4(1.0f), positions[0].position); 
    int   p0 = GetPositionIndex(animationTime); 
    int   p1 = p0 + 1; 
    float t  = GetScaleFactor(positions[p0].timeStamp, positions[p1].timeStamp, animationTime); 
    return glm::translate(glm::mat4(1.0f), 
                          glm::mix(positions[p0].position, positions[p1].position, t)); 
} 
glm::mat4 Bone::InterpolateRotation(float animationTime) const 
{ 
    if (numRotations == 0) return glm::mat4(1.0f); 
    if (numRotations == 1) return glm::mat4_cast(glm::normalize(rotations[0].orientation)); 
    int   p0 = GetRotationIndex(animationTime); 
    int   p1 = p0 + 1; 
    float t  = GetScaleFactor(rotations[p0].timeStamp, rotations[p1].timeStamp, animationTime); 
    glm::quat finalRot = glm::slerp(rotations[p0].orientation, rotations[p1].orientation, t); 
    return glm::mat4_cast(glm::normalize(finalRot)); 
} 
glm::mat4 Bone::InterpolateScaling(float animationTime) const 
{ 
    if (numScalings == 0) return glm::mat4(1.0f); 
    if (numScalings == 1) return glm::scale(glm::mat4(1.0f), scales[0].scale); 
    int   p0 = GetScaleIndex(animationTime); 
    int   p1 = p0 + 1; 
    float t  = GetScaleFactor(scales[p0].timeStamp, scales[p1].timeStamp, animationTime); 
    return glm::scale(glm::mat4(1.0f), 
                      glm::mix(scales[p0].scale, scales[p1].scale, t)); 
}