#pragma once 

#include <string> 

#include <vector> 

#include <unordered_map> 

#include <assimp/scene.h> 

#include <glm/glm.hpp> 

#include "bone.h" 

class Model; 

struct AssimpNodeData 

{    

    glm::mat4              transformation{ 1.0f }; 

    std::string            name; 

    int                    childrenCount = 0; 

    std::vector<AssimpNodeData> children; 

};

struct BoneInfo 

{ 

    int       id     = -1;    

    glm::mat4 offset{ 1.0f };  

};          

class Animation  

{ 

public:     

    Animation() = default;

    Animation(const aiScene* scene, Model* model, unsigned int animationIndex = 0); 

    Bone* FindBone(const std::string& name); 

      

    float               GetDuration()              const { return duration; } 

    float               GetTicksPerSecond()        const { return ticksPerSecond; } 

    const AssimpNodeData& GetRootNode()            const { return rootNode; } 

    const glm::mat4&    GetGlobalInverseTransform()const { return globalInverseTransform; } 

    const std::unordered_map<std::string, BoneInfo>& GetBoneIDMap() const { return boneInfoMap; } 

    const std::string&  GetName()                  const { return name; } 

private: 

    float         duration        = 0.0f; 

    float         ticksPerSecond  = 25.0f; 

    std::string   name; 

    glm::mat4     globalInverseTransform{ 1.0f }; 

    std::vector<Bone>      bones;         

    AssimpNodeData         rootNode; 

    std::unordered_map<std::string, BoneInfo> boneInfoMap; 

    std::unordered_map<std::string, Bone*> boneNameMap; 

    void ReadMissingBones(const aiAnimation* animation, Model& model); 

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src); 

    void BuildBoneNameMap(); 

};