#include "model_loader.h"
#include "../shader.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stb_image.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <btBulletDynamicsCommon.h>
#include <glm/gtx/quaternion.hpp>
#include <set>
#include <unordered_map>

using namespace std;

bool IsColliderMesh(const std::string &name) {
    return name.rfind("Collider_", 0) == 0;
}


static unsigned int GetDefaultWhiteTexture() {
    static unsigned int whiteTexID = 0;
    if (whiteTexID == 0) {
        unsigned char whitePixel[] = {255, 255, 255, 255};
        glGenTextures(1, &whiteTexID);
        glBindTexture(GL_TEXTURE_2D, whiteTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    return whiteTexID;
}
static unsigned int GetDefaultBlackTexture() {
    static unsigned int blackTexID = 0;
    if (blackTexID == 0) {
        unsigned char px[4] = {0, 0, 0, 255};
        glGenTextures(1, &blackTexID);
        glBindTexture(GL_TEXTURE_2D, blackTexID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    return blackTexID;
}
static texture MakeWhiteFallback(const std::string &typeName) {
    texture t;
    t.id = GetDefaultWhiteTexture();
    t.type = typeName;
    t.path = "__white_fallback__";
    return t;
}


unsigned int TextureFromEmbeddedData(const aiTexel *data, unsigned int width, unsigned int height, bool isSRGB = false) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLenum internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return textureID;
}
unsigned int TextureFromFile(const char *path, const string &directory, const aiScene *scene = nullptr, bool isSRGB = false) {

    string filename = string(path);

    if (filename[0] == '*') {
        if (!scene) {
            std::cerr << "Embedded texture reference but no scene provided: " << filename << std::endl;
            return 0;
        }

        int textureIndex = std::atoi(filename.c_str() + 1);
        if (textureIndex < 0 || textureIndex >= static_cast<int>(scene->mNumTextures)) {
            std::cerr << "Invalid embedded texture index: " << textureIndex << std::endl;
            return 0;
        }

        aiTexture *embeddedTexture = scene->mTextures[textureIndex];

        if (embeddedTexture->mHeight == 0) {
            int width, height, nrComponents;
            unsigned char *data = stbi_load_from_memory(reinterpret_cast<unsigned char *>(embeddedTexture->pcData), embeddedTexture->mWidth, &width, &height, &nrComponents, 4);

            if (data) {
                unsigned int textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                GLenum internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                stbi_image_free(data);
                return textureID;

            } else {
                std::cerr << "Failed to decompress embedded texture: " << filename << std::endl;
                return 0;
            }

        } else {
            return TextureFromEmbeddedData(reinterpret_cast<aiTexel *>(embeddedTexture->pcData), embeddedTexture->mWidth, embeddedTexture->mHeight, isSRGB);
        }
    }

    string fullPath = directory + '/' + filename;
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;

    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);

    if (data && width > 0 && height > 0) {
        GLenum internalFormat, dataFormat;

        if (nrComponents == 1) {
            internalFormat = GL_RED;
            dataFormat = GL_RED;

        } else if (nrComponents == 3) {
            internalFormat = isSRGB ? GL_SRGB8 : GL_RGB;
            dataFormat = GL_RGB;

        } else {
            internalFormat = isSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);

    } else {

        if (data)
            stbi_image_free(data);
    }
    return textureID;
}


Mesh::Mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    setupMesh();
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, texCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(vertex),
                           (void *)offsetof(vertex, boneIDs));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, weights));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, tangent));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, bitangent));
    glBindVertexArray(0);
}


Model::Model(const std::string &path) {
    loadModel(path);
}
void Model::draw(Shader &shader) {
    draw(shader, glm::mat4(1.0f));
}

void Model::draw(Shader &shader, const glm::mat4 &parentTransform, const std::vector<glm::mat4> &boneMatrices, bool renderColliders) {

    bool isAnimated = !boneMatrices.empty();

    if (isAnimated) {
        shader.setBool("hasAnimation", true);

        static std::unordered_map<unsigned int, std::vector<GLint>> boneLocationCache;
        auto &locations = boneLocationCache[shader.ID];

        if (locations.size() != boneMatrices.size()) {
            locations.resize(boneMatrices.size());

            for (size_t i = 0; i < boneMatrices.size(); i++) {
                std::string name = "finalBonesMatrices[" + std::to_string(i) + "]";
                locations[i] = glGetUniformLocation(shader.ID, name.c_str());
            }
        }

        for (size_t i = 0; i < boneMatrices.size(); i++)
            glUniformMatrix4fv(locations[i], 1, GL_FALSE, &boneMatrices[i][0][0]);

    } else {
        shader.setBool("hasAnimation", false);
    }

    draw(shader, parentTransform, renderColliders, isAnimated);
}
void Model::draw(Shader &shader, const glm::mat4 &parentTransform, bool renderColliders, bool skipMeshTransform) {

    for (auto &meshInstance : meshes) {
        if (IsColliderMesh(meshInstance.name))
            continue;
        glm::mat4 modelMatrix = skipMeshTransform
                                    ? parentTransform
                                    : parentTransform * meshInstance.transform;
        shader.setMat4("model", modelMatrix);
        meshInstance.mesh.draw(shader);
    }
}
void Model::draw(Shader &shader, const glm::mat4 &parentTransform, bool renderColliders) {

    for (auto &meshInstance : meshes) {

        if (IsColliderMesh(meshInstance.name))
            continue;
        shader.setMat4(
            "model",
            parentTransform * meshInstance.transform);
        meshInstance.mesh.draw(shader);
    }
}
void Mesh::draw(Shader &shader) {
    shader.setVec3("materialBaseColor", material.baseColor);
    shader.setFloat("materialMetallic", material.metallic);
    shader.setFloat("materialRoughness", material.roughness);
    shader.setFloat("materialAO", material.ao);

    shader.setVec3("materialEmissive", material.emissive);
    shader.setFloat("emissiveStrength", material.emissiveIntensity);

    bindPBRTextures(shader);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}



void SetVertexBoneData(vertex &vert, int boneID, float weight) {

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if (vert.boneIDs[i] == -1) {
            vert.boneIDs[i] = boneID;
            vert.weights[i] = weight;
            return;
        }
    }

    std::cerr << "Warning: More than " << MAX_BONE_INFLUENCE << " bones influencing a vertex. Extra influences will be ignored." << std::endl;
}


btCollisionShape *Model::buildConvexHullCollider() {

    btConvexHullShape *convexHull = new btConvexHullShape();

    for (const auto &meshInstance : meshes) {
        if (!IsColliderMesh(meshInstance.name))
            continue;

        for (const auto &vertex : meshInstance.mesh.vertices) {

            glm::vec4 transformedPos = meshInstance.transform * glm::vec4(vertex.position, 1.0f);
            convexHull->addPoint(btVector3(transformedPos.x, transformedPos.y, transformedPos.z));
        }
    }
    convexHull->optimizeConvexHull();
    convexHull->initializePolyhedralFeatures();
    convexHull->recalcLocalAabb();
    return convexHull;
}

btCollisionShape *Model::buildCompoundBoxCollider() {
    btCompoundShape *compound = new btCompoundShape();
    for (const auto &meshInstance : meshes) {
        if (meshInstance.mesh.vertices.empty())
            continue;
        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        for (const auto &vertex : meshInstance.mesh.vertices) {
            minBounds = glm::min(minBounds, vertex.position);
            maxBounds = glm::max(maxBounds, vertex.position);
        }
        glm::vec3 center = (minBounds + maxBounds) * 0.5f;
        glm::vec3 halfExtents = (maxBounds - minBounds) * 0.5f;
        halfExtents = glm::max(halfExtents, glm::vec3(0.01f));

        glm::mat4 transform = meshInstance.transform;

        glm::vec3 translation = glm::vec3(transform[3]);
        glm::vec3 col0 = glm::vec3(transform[0]);
        glm::vec3 col1 = glm::vec3(transform[1]);
        glm::vec3 col2 = glm::vec3(transform[2]);
        glm::vec3 scale(
            glm::length(col0),
            glm::length(col1),
            glm::length(col2));

        glm::mat3 rotMat;
        rotMat[0] = (scale.x > 0.0f) ? (col0 / scale.x) : col0;
        rotMat[1] = (scale.y > 0.0f) ? (col1 / scale.y) : col1;
        rotMat[2] = (scale.z > 0.0f) ? (col2 / scale.z) : col2;
        glm::quat rotation = glm::quat_cast(rotMat);

        glm::vec3 absScale = glm::abs(scale);
        glm::vec3 scaledHalfExtents = halfExtents * absScale;
        btBoxShape *boxShape = new btBoxShape(
            btVector3(
                scaledHalfExtents.x,
                scaledHalfExtents.y,
                scaledHalfExtents.z));

        glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(center, 1.0f));
        btTransform localTransform;
        localTransform.setIdentity();
        localTransform.setOrigin(
            btVector3(worldCenter.x, worldCenter.y, worldCenter.z));
        localTransform.setRotation(
            btQuaternion(
                rotation.x,
                rotation.y,
                rotation.z,
                rotation.w));
        compound->addChildShape(localTransform, boxShape);
    }
    return compound;
}

btCollisionShape *Model::buildTriangleMeshCollider() {
    btTriangleMesh *triangleMesh = new btTriangleMesh();
    for (const auto &meshInstance : meshes) {

        const auto &vertices = meshInstance.mesh.vertices;
        const auto &indices = meshInstance.mesh.indices;

        for (size_t i = 0; i < indices.size(); i += 3) {
            glm::vec3 v0 =
                glm::vec3(
                    meshInstance.transform *
                    glm::vec4(vertices[indices[i]].position, 1.0f));
            glm::vec3 v1 =
                glm::vec3(
                    meshInstance.transform *
                    glm::vec4(vertices[indices[i + 1]].position, 1.0f));
            glm::vec3 v2 =
                glm::vec3(
                    meshInstance.transform *
                    glm::vec4(vertices[indices[i + 2]].position, 1.0f));
            triangleMesh->addTriangle(
                btVector3(v0.x, v0.y, v0.z),
                btVector3(v1.x, v1.y, v1.z),
                btVector3(v2.x, v2.y, v2.z));
        }
    }
    bool useQuantizedAabbCompression = true;
    return new btBvhTriangleMeshShape(
        triangleMesh,
        useQuantizedAabbCompression);
}

btCollisionShape *Model::buildSphericalHullCollider() {
    btCompoundShape *compound = new btCompoundShape();
    for (const auto &meshInstance : meshes) {
        if (!IsColliderMesh(meshInstance.name))
            continue;
        if (meshInstance.mesh.vertices.empty())
            continue;

        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);
        for (const auto &vertex : meshInstance.mesh.vertices) {
            minBounds = glm::min(minBounds, vertex.position);
            maxBounds = glm::max(maxBounds, vertex.position);
        }

        glm::vec3 center = (minBounds + maxBounds) * 0.5f;

        glm::vec3 halfExtents = (maxBounds - minBounds) * 0.5f;
        float maxRadius = glm::length(halfExtents);

        maxRadius = glm::max(maxRadius, 0.01f);
        btSphereShape *sphereShape = new btSphereShape(maxRadius);

        glm::mat4 transform = meshInstance.transform;

        glm::vec3 position = glm::vec3(transform * glm::vec4(center, 1.0f));
        glm::quat rotation = glm::quat_cast(transform);
        btTransform localTransform;
        localTransform.setIdentity();
        localTransform.setOrigin(
            btVector3(position.x, position.y, position.z));
        localTransform.setRotation(
            btQuaternion(
                rotation.x,
                rotation.y,
                rotation.z,
                rotation.w));
        compound->addChildShape(localTransform, sphereShape);
    }
    return compound;
}

btCollisionShape *Model::buildCapsuleColliderFromMesh() {
    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);
    bool foundAny = false;

    for (const auto &meshInstance : meshes) {
        if (!IsColliderMesh(meshInstance.name))
            continue;
        for (const auto &vertex : meshInstance.mesh.vertices) {

            glm::vec3 worldPos = glm::vec3(meshInstance.transform * glm::vec4(vertex.position, 1.0f));
            minBounds = glm::min(minBounds, worldPos);
            maxBounds = glm::max(maxBounds, worldPos);
            foundAny = true;
        }
    }
    if (!foundAny)
        return nullptr;

    glm::vec3 extents = maxBounds - minBounds;
    float radius = glm::max(extents.x, extents.z) * 0.5f;
    float height = glm::max(0.0f, extents.y - (radius * 2.0f));

    btCapsuleShape *capsule = new btCapsuleShape(radius, height);

    return capsule;
}


size_t ModelLoader::getModelCount() const { return models.size(); }
ModelLoader::ModelData &ModelLoader::getModel(size_t index) { return models.at(index); }
const ModelLoader::ModelData &ModelLoader::getModel(size_t index) const { return models.at(index); }
glm::vec3 ModelLoader::getModelScale(size_t modelIndex) const {
    if (modelIndex >= models.size())
        return glm::vec3(1.0f);
    const glm::mat4 &m = models[modelIndex].transform;
    return glm::vec3(
        glm::length(glm::vec3(m[0])),
        glm::length(glm::vec3(m[1])),
        glm::length(glm::vec3(m[2])));
}



void Model::loadModel(const std::string &path) {
    importer = std::make_unique<Assimp::Importer>();
    unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                         aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
    const aiScene *scene = importer->ReadFile(path, flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer->GetErrorString() << std::endl;
        return;
    }
    scene_ptr = scene;

    if (scene->HasAnimations()) {
        std::cout << "Animations: "
                  << scene->mNumAnimations
                  << std::endl;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
    computeBounds();
}

void Model::computeBounds() {
    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);
    bool foundAny = false;

    for (auto &meshInstance : meshes) {
        if (IsColliderMesh(meshInstance.name))
            continue;

        for (auto &vertex : meshInstance.mesh.vertices) {
            glm::vec3 worldPos = glm::vec3(meshInstance.transform * glm::vec4(vertex.position, 1.0f));
            minBounds = glm::min(minBounds, worldPos);
            maxBounds = glm::max(maxBounds, worldPos);
            foundAny = true;
        }
    }

    if (!foundAny) {

        boundsCenter = glm::vec3(0.0f);
        boundsRadius = 0.01f;
        return;
    }
    boundsCenter = (minBounds + maxBounds) * 0.5f;
    boundsRadius = glm::max(glm::length(maxBounds - boundsCenter), 0.01f);
}

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4 &from) {
    glm::mat4 to;
    to[0][0] = from.a1;
    to[0][1] = from.b1;
    to[0][2] = from.c1;
    to[0][3] = from.d1;
    to[1][0] = from.a2;
    to[1][1] = from.b2;
    to[1][2] = from.c2;
    to[1][3] = from.d2;
    to[2][0] = from.a3;
    to[2][1] = from.b3;
    to[2][2] = from.c3;
    to[2][3] = from.d3;
    to[3][0] = from.a4;
    to[3][1] = from.b4;
    to[3][2] = from.c4;
    to[3][3] = from.d4;
    return to;
}

void Model::processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform) {
    glm::mat4 transform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        MeshInstance instance;
        instance.mesh = processMesh(mesh, scene);
        instance.transform = transform;
        instance.name = node->mName.C_Str();
        meshes.push_back(instance);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, transform);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<texture> textures;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertex v;
        v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) {
            v.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }

        vertices.push_back(v);
    }
    ExtractTangentBitangent(vertices, mesh);
    ExtractBoneWeights(vertices, mesh);
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
    aiColor4D baseColorFactor(1.f, 1.f, 1.f, 1.f);
    if (mat->Get(AI_MATKEY_BASE_COLOR, baseColorFactor) != AI_SUCCESS)
        mat->Get(AI_MATKEY_COLOR_DIFFUSE, baseColorFactor);
    float metallicFactor = 0.0f;
    float roughnessFactor = 0.5f;
    mat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor);
    mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor);
    std::vector<texture> diffuseMaps = loadMaterialTextures(mat, aiTextureType_BASE_COLOR, "diffuse");
    if (diffuseMaps.empty())
        diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "diffuse");
    if (diffuseMaps.empty()) {

        unsigned char r = static_cast<unsigned char>(glm::clamp(baseColorFactor.r, 0.f, 1.f) * 255.f);
        unsigned char g = static_cast<unsigned char>(glm::clamp(baseColorFactor.g, 0.f, 1.f) * 255.f);
        unsigned char b = static_cast<unsigned char>(glm::clamp(baseColorFactor.b, 0.f, 1.f) * 255.f);
        unsigned char a = static_cast<unsigned char>(glm::clamp(baseColorFactor.a, 0.f, 1.f) * 255.f);
        unsigned char px[4] = {r, g, b, a};
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture t;
        t.id = id;
        t.type = "diffuse";
        t.path = "__color_fallback__";
        diffuseMaps.push_back(t);
    }
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::set<std::string> diffusePaths;
    for (auto &t : diffuseMaps)
        diffusePaths.insert(t.path);
    std::vector<texture> normalMaps = loadMaterialTextures(mat, aiTextureType_NORMALS, "normal");
    if (normalMaps.empty())
        normalMaps = loadMaterialTextures(mat, aiTextureType_NORMAL_CAMERA, "normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<texture> mrMaps;
    if (mat->GetTextureCount(aiTextureType_UNKNOWN) > 0) {
        aiString unknownPath;
        mat->GetTexture(aiTextureType_UNKNOWN, 0, &unknownPath);
        if (diffusePaths.find(unknownPath.C_Str()) == diffusePaths.end())
            mrMaps = loadMaterialTextures(mat, aiTextureType_UNKNOWN, "metallicRoughness");
    }
    if (mrMaps.empty())
        mrMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE_ROUGHNESS, "metallicRoughness");
    if (mrMaps.empty())
        mrMaps = loadMaterialTextures(mat, aiTextureType_METALNESS, "metallicRoughness");
    if (mrMaps.empty()) {

        unsigned char px[4] = {
            0,
            static_cast<unsigned char>(glm::clamp(roughnessFactor, 0.f, 1.f) * 255.f),
            static_cast<unsigned char>(glm::clamp(metallicFactor, 0.f, 1.f) * 255.f),
            255};
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture t;
        t.id = id;
        t.type = "metallicRoughness";
        t.path = "__scalar_mr_fallback__";
        mrMaps.push_back(t);
    }
    textures.insert(textures.end(), mrMaps.begin(), mrMaps.end());
    std::vector<texture> aoMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT_OCCLUSION, "ao");
    if (aoMaps.empty())
        aoMaps = loadMaterialTextures(mat, aiTextureType_LIGHTMAP, "ao");
    if (aoMaps.empty())
        aoMaps.push_back(MakeWhiteFallback("ao"));
    textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    aiColor3D emissiveFactor(0.f, 0.f, 0.f);
    mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveFactor);
    float emissiveIntensity = 1.0f;
    mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveIntensity);
    std::vector<texture> emissiveMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, "emissive");
    if (emissiveMaps.empty()) {
        unsigned char r = static_cast<unsigned char>(glm::clamp(emissiveFactor.r, 0.f, 1.f) * 255.f);
        unsigned char g = static_cast<unsigned char>(glm::clamp(emissiveFactor.g, 0.f, 1.f) * 255.f);
        unsigned char b = static_cast<unsigned char>(glm::clamp(emissiveFactor.b, 0.f, 1.f) * 255.f);
        unsigned char px[4] = {r, g, b, 255};
        unsigned int id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        texture t;
        t.id = id;
        t.type = "emissive";
        t.path = "__emissive_fallback__";
        emissiveMaps.push_back(t);
    }
    textures.insert(textures.end(), emissiveMaps.begin(), emissiveMaps.end());
    std::cout << "\n=== MATERIAL (" << mesh->mName.C_Str() << ") ===\n";
    std::cout << "  baseColor=(" << baseColorFactor.r << "," << baseColorFactor.g << "," << baseColorFactor.b << ")"
              << "  metallic=" << metallicFactor << "  roughness=" << roughnessFactor << "\n";
    for (auto &t : textures)
        std::cout << "  " << t.type << " -> " << t.path << "\n";
    auto isRealTexture = [](const std::vector<texture> &maps) -> bool {
        return !maps.empty() && maps[0].path.rfind("__", 0) != 0;
    };

    Mesh result_mesh(vertices, indices, textures);

    result_mesh.material.name = mesh->mName.C_Str();

    result_mesh.material.metallic = metallicFactor;
    result_mesh.material.roughness = roughnessFactor;

    result_mesh.material.baseColor = glm::vec3(1.0f);

    if (isRealTexture(emissiveMaps))
        result_mesh.material.emissive = glm::vec3(1.0f);
    else
        result_mesh.material.emissive = glm::vec3(
            emissiveFactor.r, emissiveFactor.g, emissiveFactor.b);
    result_mesh.material.emissiveIntensity = emissiveIntensity;

    result_mesh.material.hasNormalMap = isRealTexture(normalMaps);
    result_mesh.material.hasMetallicRoughnessMap = isRealTexture(mrMaps);
    result_mesh.material.hasAOMap = isRealTexture(aoMaps);
    result_mesh.material.hasEmissiveMap = isRealTexture(emissiveMaps);

    for (const auto &t : textures) {
        if (t.type == "diffuse")
            result_mesh.material.albedoTexID = t.id;
        else if (t.type == "normal")
            result_mesh.material.normalTexID = t.id;
        else if (t.type == "metallicRoughness")
            result_mesh.material.metallicRoughnessTexID = t.id;
        else if (t.type == "ao")
            result_mesh.material.aoTexID = t.id;
        else if (t.type == "emissive")
            result_mesh.material.emissiveTexID = t.id;
    }

    return result_mesh;
}

void Model::ExtractBoneWeights(std::vector<vertex> &vertices, aiMesh *mesh) {
    for (unsigned int i = 0; i < mesh->mNumBones; i++) {
        aiBone *bone = mesh->mBones[i];
        std::string boneName(bone->mName.C_Str());

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo boneInfo;
            boneInfo.id = boneCounter++;
            boneInfo.offset = aiMatrix4x4ToGlm(bone->mOffsetMatrix);
            boneInfoMap[boneName] = boneInfo;
        }

        int boneID = boneInfoMap[boneName].id;

        for (unsigned int j = 0; j < bone->mNumWeights; j++) {
            unsigned int vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;
            SetVertexBoneData(vertices[vertexID], boneID, weight);
        }
    }
}
void Mesh::bindPBRTextures(Shader &shader) {

    shader.setBool("hasNormalMap", false);
    shader.setBool("hasMetallicRoughnessMap", false);
    shader.setBool("hasAOMap", false);
    shader.setBool("hasEmissiveMap", false);

    bool filledDiffuse = false;
    bool filledMR = false;
    bool filledAO = false;
    bool filledEmissive = false;

    for (unsigned int i = 0; i < textures.size(); i++) {
        const std::string &type = textures[i].type;
        int unit = -1;

        if (type == "diffuse" && material.useAlbedoMap) {
            unit = 0;
            shader.setInt("texture_diffuse1", unit);
            filledDiffuse = true;
        } else if (type == "normal" && material.useNormalMap && material.hasNormalMap) {
            unit = 5;
            shader.setInt("texture_normal1", unit);
            shader.setBool("hasNormalMap", true);
        } else if (type == "metallicRoughness" && material.useMetallicRoughnessMap && material.hasMetallicRoughnessMap) {
            unit = 6;
            shader.setInt("texture_metallicRoughness1", unit);
            shader.setBool("hasMetallicRoughnessMap", true);
            filledMR = true;
        } else if (type == "ao" && material.useAOMap && material.hasAOMap) {
            unit = 7;
            shader.setInt("texture_ao1", unit);
            shader.setBool("hasAOMap", true);
            filledAO = true;
        } else if (type == "emissive" && material.useEmissiveMap && material.hasEmissiveMap) {
            unit = 8;
            shader.setInt("texture_emissive1", unit);
            shader.setBool("hasEmissiveMap", true);
            filledEmissive = true;
        } else {
            continue;
        }

        if (unit >= 0) {
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
    }

    unsigned int white = GetDefaultWhiteTexture();

    if (!filledDiffuse) {
        shader.setInt("texture_diffuse1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, white);
    }
    if (!filledMR) {
        shader.setInt("texture_metallicRoughness1", 6);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, white);
    }
    if (!filledAO) {
        shader.setInt("texture_ao1", 7);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, white);
    }
    if (!filledEmissive) {
        shader.setInt("texture_emissive1", 8);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, GetDefaultBlackTexture());
    }
}

void Model::ExtractTangentBitangent(std::vector<vertex> &vertices, aiMesh *mesh) {
    if (mesh->HasTangentsAndBitangents()) {
        for (unsigned int i = 0; i < vertices.size(); i++) {
            vertices[i].tangent = glm::vec3(
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z);
            vertices[i].bitangent = glm::vec3(
                mesh->mBitangents[i].x,
                mesh->mBitangents[i].y,
                mesh->mBitangents[i].z);
        }
    } else {

        for (unsigned int i = 0; i < vertices.size(); i += 3) {
            glm::vec3 edge1 = vertices[i + 1].position - vertices[i].position;
            glm::vec3 edge2 = vertices[i + 2].position - vertices[i].position;

            glm::vec2 deltaUV1 = vertices[i + 1].texCoords - vertices[i].texCoords;
            glm::vec2 deltaUV2 = vertices[i + 2].texCoords - vertices[i].texCoords;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y + 0.0001f);

            glm::vec3 tangent(
                f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
                f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
                f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z));

            for (unsigned int j = 0; j < 3; j++) {
                vertices[i + j].tangent = glm::normalize(tangent);
                vertices[i + j].bitangent =
                    glm::normalize(glm::cross(vertices[i + j].normal, tangent));
            }
        }
    }
}


std::vector<texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName) {
    std::vector<texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
            std::cout
                << "Loading "
                << typeName
                << " : "
                << str.C_Str()
                << std::endl;
        }
        if (!skip) {
            texture texture;
            bool isSRGB = (typeName == "diffuse");
            texture.id = TextureFromFile(str.C_Str(), directory, scene_ptr, isSRGB);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }
    return textures;
}

void ModelLoader::loadModel(const std::string &modelPath, const glm::vec3 &position, const glm::vec3 &scale) {

    try {
        std::shared_ptr<Model> modelPtr;

        if (modelCache.find(modelPath) != modelCache.end()) {
            modelPtr = modelCache[modelPath];
        } else {
            modelPtr = std::make_shared<Model>(modelPath);
            modelCache[modelPath] = modelPtr;
        }
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix =
            glm::translate(modelMatrix, position);
        modelMatrix =
            glm::scale(modelMatrix, scale);
        models.push_back({modelPtr,
                          modelMatrix});
    } catch (const std::exception &e) {
        std::cerr
            << "Failed to load model '"
            << modelPath
            << "': "
            << e.what()
            << std::endl;
    }
}

void ModelLoader::clearModels() {
    models.clear();
    std::cout << "All models cleared" << std::endl;
}

void ModelLoader::setModelTransform(size_t modelIndex, const glm::vec3 &position, const glm::vec3 &scale, float rotationAngle, const glm::vec3 &rotationAxis) {
    if (modelIndex >= models.size()) {
        std::cerr << "Invalid model index: " << modelIndex << std::endl;
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, rotationAxis);
    modelMatrix = glm::scale(modelMatrix, scale);

    models[modelIndex].transform = modelMatrix;
}

void ModelLoader::updateModelTransform(size_t modelIndex, const glm::vec3 &deltaPosition, const glm::vec3 &deltaScale, float deltaRotationAngle, const glm::vec3 &rotationAxis) {
    if (modelIndex >= models.size()) {
        std::cerr << "Invalid model index: " << modelIndex << std::endl;
        return;
    }

    glm::mat4 &modelMatrix = models[modelIndex].transform;

    modelMatrix = glm::translate(modelMatrix, deltaPosition);

    modelMatrix = glm::rotate(modelMatrix, deltaRotationAngle, rotationAxis);

    modelMatrix = glm::scale(modelMatrix, deltaScale);
}

glm::vec3 ModelLoader::getModelPosition(size_t modelIndex) const {
    if (modelIndex >= models.size()) {
        std::cerr << "Invalid model index: " << modelIndex << std::endl;
        return glm::vec3(0.0f);
    }

    return glm::vec3(models[modelIndex].transform[3]);
}

void ModelLoader::setModelAnimation(size_t modelIndex, unsigned int animationIndex) {
    if (modelIndex >= models.size()) {
        std::cerr << "ModelLoader::setModelAnimation: invalid model index " << modelIndex << std::endl;
        return;
    }
    ModelData &data = models[modelIndex];
    const aiScene *scene = data.model->getScene();
    if (!scene || !scene->HasAnimations() || animationIndex >= scene->mNumAnimations) {
        std::cerr << "ModelLoader::setModelAnimation: model at index " << modelIndex
                  << " has no animation at index " << animationIndex << std::endl;
        return;
    }
    data.animation = std::make_unique<Animation>(scene, data.model.get(), animationIndex);
    data.animator = std::make_unique<Animator>();
    data.animator->PlayAnimation(data.animation.get());
}

void ModelLoader::updateAnimations(float deltaTime) {
    for (auto &data : models) {
        if (data.animator)
            data.animator->UpdateAnimation(deltaTime);
    }
}

bool ModelLoader::hasAnimation(size_t modelIndex) const {
    return modelIndex < models.size() && models[modelIndex].animator != nullptr;
}

void ModelLoader::blendModelAnimations(size_t modelIndex, const std::vector<std::pair<unsigned int, float>> &layers) {

    if (modelIndex >= models.size()) {
        std::cerr << "ModelLoader::blendModelAnimations: invalid model index "
                  << modelIndex << std::endl;
        return;
    }

    if (layers.empty()) {
        std::cerr << "ModelLoader::blendModelAnimations: layer list is empty, "
                     "ignoring call."
                  << std::endl;
        return;
    }

    ModelData &data = models[modelIndex];
    const aiScene *scene = data.model->getScene();

    if (!scene || !scene->HasAnimations()) {
        std::cerr << "ModelLoader::blendModelAnimations: model at index "
                  << modelIndex << " has no animations." << std::endl;
        return;
    }

    data.blendAnimations.clear();
    data.blendAnimations.reserve(layers.size());

    if (!data.animator)
        data.animator = std::make_unique<Animator>();

    std::vector<BlendLayer> animatorLayers;
    animatorLayers.reserve(layers.size());

    for (const auto &[animIndex, weight] : layers) {
        if (animIndex >= scene->mNumAnimations) {
            std::cerr << "ModelLoader::blendModelAnimations: animation index "
                      << animIndex << " out of range (model has "
                      << scene->mNumAnimations << " animations). "
                                                  "Skipping this layer."
                      << std::endl;
            continue;
        }

        data.blendAnimations.push_back(
            std::make_unique<Animation>(scene, data.model.get(), animIndex));

        BlendLayer layer;
        layer.animation = data.blendAnimations.back().get();
        layer.weight = weight;
        layer.time = 0.0f;

        animatorLayers.push_back(layer);
    }

    if (animatorLayers.empty()) {
        std::cerr << "ModelLoader::blendModelAnimations: no valid layers "
                     "could be built."
                  << std::endl;
        return;
    }

    data.animator->SetBlendLayers(std::move(animatorLayers));

    data.animation.reset(
        new Animation(scene, data.model.get(),
                      layers[0].first));
}

void ModelLoader::setBlendWeights(size_t modelIndex, const std::vector<float> &weights) {
    if (modelIndex >= models.size() || !models[modelIndex].animator)
        return;

    Animator &animator = *models[modelIndex].animator;
    for (size_t i = 0; i < weights.size(); i++)
        animator.SetLayerWeight(i, weights[i]);
}

const std::vector<glm::mat4> &ModelLoader::getBoneMatrices(size_t modelIndex) const {
    static const std::vector<glm::mat4> empty;
    if (modelIndex < models.size() && models[modelIndex].animator)
        return models[modelIndex].animator->GetFinalBoneMatrices();
    return empty;
}