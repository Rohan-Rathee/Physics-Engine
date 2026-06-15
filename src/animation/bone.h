#ifndef BONE_H
#define BONE_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <assimp/scene.h>






struct BoneInfo
{
    int id;
    glm::mat4 offset;
};

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);


    void Update(float animationTime);

    glm::mat4 GetLocalTransform() const { return localTransform; }
    const std::string& GetBoneName() const { return name; }
    int GetBoneID() const { return id; }

private:
    int GetPositionIndex(float animationTime) const;
    int GetRotationIndex(float animationTime) const;
    int GetScaleIndex(float animationTime) const;

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) const;

    glm::mat4 InterpolatePosition(float animationTime) const;
    glm::mat4 InterpolateRotation(float animationTime) const;
    glm::mat4 InterpolateScaling(float animationTime) const;

    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
    int numPositions = 0;
    int numRotations = 0;
    int numScalings = 0;

    glm::mat4 localTransform = glm::mat4(1.0f);
    std::string name;
    int id;
};

#endif
