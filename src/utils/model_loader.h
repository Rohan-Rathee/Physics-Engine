#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

// Forward declaration
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

    Mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures);
    void draw(Shader& shader);

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

class Model {
public:
    Model(const std::string& path);
    void draw(Shader& shader);
private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<texture> textures_loaded;
    const aiScene* scene_ptr;
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};

#endif
