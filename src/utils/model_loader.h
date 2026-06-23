#ifndef MODEL_LOADER_H 
#define MODEL_LOADER_H 
#include <assimp/Importer.hpp> 
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 
#include <glm/glm.hpp> 
#include <vector> 
#include <string> 
#include <memory> 
#include <unordered_map> 
#include "../animation/bone.h" 
#include "../animation/animator.h" 
#include "../animation/animation.h" 
class btCollisionShape; 
  
class Shader; 
const int MAX_BONE_INFLUENCE = 4; 
struct vertex 
{ 
    glm::vec3 position; 
    glm::vec3 normal; 
    glm::vec2 texCoords; 
    int boneIDs[MAX_BONE_INFLUENCE]; 
    float weights[MAX_BONE_INFLUENCE]; 
    vertex() 
    { 
        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) 
        { 
            boneIDs[i] = -1; 
            weights[i] = 0.0f; 
        } 
    } 
}; 
  
  
  
struct texture 
{ 
    unsigned int id; 
    std::string type; 
    std::string path; 
}; 
class Mesh 
{ 
public: 
    std::vector<vertex> vertices; 
    std::vector<unsigned int> indices; 
    std::vector<texture> textures; 
    Mesh() = default; 
    Mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures); 
    void draw(Shader &shader); 
private: 
    unsigned int VAO, VBO, EBO; 
    void setupMesh(); 
}; 
struct MeshInstance 
{ 
    Mesh mesh; 
    glm::mat4 transform; 
    std::string name;   
}; 
class Model 
{ 
public: 
    btCollisionShape *buildConvexHullCollider(); 
    btCollisionShape *buildCompoundBoxCollider(); 
    btCollisionShape *buildTriangleMeshCollider(); 
    btCollisionShape *buildSphericalHullCollider(); 
    btCollisionShape *buildCapsuleColliderFromMesh(); 
    Model(const std::string &path); 
    void draw(Shader &shader); 
    void draw(Shader &shader, const glm::mat4 &parentTransform, bool renderColliders = false); 
      
      
      
      
     
    void draw(Shader &shader, const glm::mat4& parentTransform, bool renderColliders, bool skipMeshTransform); 
    void draw(Shader &shader, const glm::mat4 &parentTransform, 
              const std::vector<glm::mat4> &boneMatrices, bool renderColliders = false); 
    const aiScene *getScene() const 
    { 
        return scene_ptr; 
    } 
    const std::unordered_map<std::string, BoneInfo>& getBoneInfoMap() const 
    { 
        return boneInfoMap; 
    } 
    int getBoneCount() const 
    { 
        return boneCounter; 
    } 
    glm::vec3 boundsCenter = glm::vec3(0.0f); 
    float boundsRadius = 0.01f; 
    void computeBounds(); 
private: 
    void ExtractBoneWeights(std::vector<vertex> &vertices, aiMesh *mesh); 
    std::vector<MeshInstance> meshes; 
    std::string directory; 
    std::vector<texture> textures_loaded; 
    std::unordered_map<std::string, BoneInfo> boneInfoMap;   
    std::unique_ptr<Assimp::Importer> importer; 
    int boneCounter = 0; 
    const aiScene *scene_ptr = nullptr; 
    void loadModel(const std::string &path); 
    void processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform); 
    Mesh processMesh(aiMesh *mesh, const aiScene *scene); 
    std::vector<texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName); 
}; 
class ModelLoader 
{ 
        struct ModelData    
    { 
        std::shared_ptr<Model> model;    
        glm::mat4 transform; 
        std::unique_ptr<Animation> animation; 
        std::unique_ptr<Animator> animator; 
        std::vector<std::unique_ptr<Animation>> blendAnimations; 
    }; 
public: 
    std::unordered_map<std::string, std::shared_ptr<Model>> modelCache; 
    void loadModel(const std::string &modelPath, const glm::vec3 &position = glm::vec3(0.0f), 
                   const glm::vec3 &scale = glm::vec3(1.0f)); 
    void clearModels(); 
    void setModelTransform(size_t modelIndex, const glm::vec3 &position, const glm::vec3 &scale, 
                           float rotationAngle = 0.0f, const glm::vec3 &rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f)); 
    void updateModelTransform(size_t modelIndex, const glm::vec3 &deltaPosition, const glm::vec3 &deltaScale, 
                              float deltaRotationAngle = 0.0f, const glm::vec3 &rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f)); 
      
    glm::vec3 getModelPosition(size_t modelIndex) const; 
    glm::vec3 getModelScale(size_t modelIndex) const; 
      
      
      
      
    void setModelAnimation(size_t modelIndex, unsigned int animationIndex = 0); 
    void blendModelAnimations(size_t modelIndex, const std::vector<std::pair<unsigned int /*animIndex*/, float /*weight*/>>& layers); 
      
      
    void updateAnimations(float deltaTime); 
    void setBlendWeights( 
    size_t modelIndex, 
    const std::vector<float>& weights     
); 
    bool hasAnimation(size_t modelIndex) const; 
      
      
      
    const std::vector<glm::mat4>& getBoneMatrices(size_t modelIndex) const; 
    size_t getModelCount() const; 
    ModelData& getModel(size_t index); 
    const ModelData& getModel(size_t index) const; 
private: 
    std::vector<ModelData> models; 
     
    friend class RenderSystem; 
}; 
#endif  