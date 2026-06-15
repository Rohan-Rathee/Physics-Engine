#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <string>
#include <unordered_map>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>
#include "bone.h"





class Model;



struct AssimpNodeData
{
    glm::mat4 transformation;
    
    std::string name;
    int childrenCount = 0;
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation() = default;




    Animation(const aiScene* scene, Model* model, unsigned int animationIndex = 0);

    Bone* FindBone(const std::string& name);

    float GetTicksPerSecond() const { return ticksPerSecond; }
    float GetDuration() const { return duration; }
    const std::string& GetName() const { return name; }
    const AssimpNodeData& GetRootNode() const { return rootNode; }
    const std::unordered_map<std::string, BoneInfo>& GetBoneIDMap() const { return boneInfoMap; }
    glm::mat4& GetGlobalInverseTransform() { return globalInverseTransform; }
    
private:
glm::mat4 globalInverseTransform;
    void ReadMissingBones(const aiAnimation* animation, Model& model);
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);

    float duration = 0.0f;
    float ticksPerSecond = 0.0f;
    std::vector<Bone> bones;
    AssimpNodeData rootNode;
    std::unordered_map<std::string, BoneInfo> boneInfoMap;
    std::string name;
};

#endif
