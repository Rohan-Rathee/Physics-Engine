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

class btCollisionShape;

// Forward declaration
class Shader;

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;

    int boneIDs[4] = {0};
    float weights[4] = {0.0f};
};

struct texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<texture> textures;

    Mesh() = default;
    Mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures);
    void draw(Shader& shader);

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

struct MeshInstance {
    Mesh mesh;
    glm::mat4 transform;
    std::string name; // Optional: store name for debugging or future use
};

class Model {
public:
    btCollisionShape* buildConvexHullCollider();
    btCollisionShape* buildCompoundBoxCollider();
    btCollisionShape* buildTriangleMeshCollider();
    btCollisionShape* buildSphericalHullCollider();
    btCollisionShape* buildCapsuleColliderFromMesh();
    Model(const std::string& path);
    void draw(Shader& shader);
    void draw(Shader& shader, const glm::mat4& parentTransform, bool renderColliders = false);
private:
    std::vector<MeshInstance> meshes;
    std::string directory;
    std::vector<texture> textures_loaded;
    const aiScene* scene_ptr;
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene, glm::mat4 parentTransform);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};

class ModelLoader {
public:
    std::unordered_map<std::string, std::shared_ptr<Model>> modelCache;
    void loadModel(const std::string& modelPath, const glm::vec3& position = glm::vec3(0.0f), 
                   const glm::vec3& scale = glm::vec3(1.0f));
    void clearModels();
    void setModelTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale, 
                           float rotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
    void updateModelTransform(size_t modelIndex, const glm::vec3& deltaPosition, const glm::vec3& deltaScale, 
                              float deltaRotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Get position from model's stored transform
    glm::vec3 getModelPosition(size_t modelIndex) const;
    glm::vec3 getModelScale(size_t modelIndex) const;

private:
    struct ModelData {
        std::shared_ptr<Model> model;
        glm::mat4 transform;
    };
    std::vector<ModelData> models;
    
    friend class RenderSystem;
}; 
#endif