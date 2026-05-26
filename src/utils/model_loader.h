#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>


class Shader;

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
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
};

class Model {
public:
    Model(const std::string& path);
    void draw(Shader& shader);
    void draw(Shader& shader, const glm::mat4& parentTransform);
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
    void loadModel(const std::string& modelPath, const glm::vec3& position = glm::vec3(0.0f), 
                   const glm::vec3& scale = glm::vec3(1.0f));
    void clearModels();
    void setModelTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale, 
                           float rotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
    void updateModelTransform(size_t modelIndex, const glm::vec3& deltaPosition, const glm::vec3& deltaScale, 
                              float deltaRotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));

private:
    struct ModelData {
        std::unique_ptr<Model> model;
        glm::mat4 transform;
    };
    std::vector<ModelData> models;
    
    friend class RenderSystem;
}; 
#endif
