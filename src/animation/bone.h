#pragma once 
#include <vector> 
#include <string> 
#include <glm/glm.hpp> 
#include <assimp/anim.h> 
#include <glm/ext/quaternion_float.hpp> 
#include <glm/gtc/quaternion.hpp> 
  
struct KeyPosition { 
    glm::vec3 position; 
    float     timeStamp; 
}; 
struct KeyRotation { 
    glm::quat orientation; 
    float     timeStamp; 
}; 
struct KeyScale { 
    glm::vec3 scale; 
    float     timeStamp; 
}; 
  
class Bone 
{ 
public: 
    Bone(const std::string& name, int ID, const aiNodeAnim* channel); 
      
    void Update(float animationTime); 
      
    const glm::mat4& GetLocalTransform() const { return localTransform; } 
    const std::string& GetBoneName()     const { return name; } 
    int                GetBoneID()       const { return id; } 
      
    int GetPositionKeyCount() const { return numPositions; } 
    int GetRotationKeyCount() const { return numRotations; } 
    int GetScaleKeyCount()    const { return numScalings;  } 
private: 
      
    std::vector<KeyPosition> positions; 
    std::vector<KeyRotation> rotations; 
    std::vector<KeyScale>    scales; 
    int numPositions = 0; 
    int numRotations = 0; 
    int numScalings  = 0; 
    glm::mat4   localTransform{ 1.0f }; 
    std::string name; 
    int         id = 0; 
      
      
      
      
    mutable int m_LastPosIdx   = 0; 
    mutable int m_LastRotIdx   = 0; 
    mutable int m_LastScaleIdx = 0; 
      
      
      
    int GetPositionIndex(float animationTime) const; 
    int GetRotationIndex(float animationTime) const; 
    int GetScaleIndex   (float animationTime) const; 
    float     GetScaleFactor(float last, float next, float time) const; 
    glm::mat4 InterpolatePosition(float animationTime) const; 
    glm::mat4 InterpolateRotation(float animationTime) const; 
    glm::mat4 InterpolateScaling (float animationTime) const; 
};