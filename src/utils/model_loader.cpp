#include "model_loader.h"
#include "../shader.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <btBulletDynamicsCommon.h>
using namespace std;

bool IsColliderMesh(const std::string& name)
{
    return name.rfind("Collider_", 0) == 0;
}
// Helper function to create a default white texture
unsigned int CreateWhiteTexture()
{
    unsigned char whitePixel[] = {255, 255, 255, 255}; // RGBA white
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);//
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    return textureID;
}
// Helper function to load texture from embedded data (for GLB/GLTF)
unsigned int TextureFromEmbeddedData(const aiTexel *data, unsigned int width, unsigned int height)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}
// Helper function to load texture from file
unsigned int TextureFromFile(const char *path, const string &directory, const aiScene* scene = nullptr)
{
    string filename = string(path);
    
    // Check if this is an embedded texture reference (starts with '*')
    if (filename[0] == '*')
    {
        if (!scene)
        {
            std::cerr << "Embedded texture reference but no scene provided: " << filename << std::endl;
            return 0;
        }
        
        // Extract texture index from "*N"
        int textureIndex = std::atoi(filename.c_str() + 1);
        if (textureIndex < 0 || textureIndex >= static_cast<int>(scene->mNumTextures))
        {
            std::cerr << "Invalid embedded texture index: " << textureIndex << std::endl;
            return 0;
        }
        
        aiTexture* embeddedTexture = scene->mTextures[textureIndex];
        
        if (embeddedTexture->mHeight == 0)
        {
            // Compressed texture - decompress using stbi
            int width, height, nrComponents;
            unsigned char *data = stbi_load_from_memory(
                reinterpret_cast<unsigned char*>(embeddedTexture->pcData),
                embeddedTexture->mWidth,
                &width, &height, &nrComponents, 4
            );
            
            if (data)
            {
                unsigned int textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                stbi_image_free(data);
                return textureID;
            }
            else
            {
                std::cerr << "Failed to decompress embedded texture: " << filename << std::endl;
                return 0;
            }
        }
        else
        {
            // Uncompressed texture (RGBA)
            unsigned int textureID = TextureFromEmbeddedData(
                reinterpret_cast<aiTexel*>(embeddedTexture->pcData),
                embeddedTexture->mWidth,
                embeddedTexture->mHeight
            );
            return textureID;
        }
    }
    
    // Regular file-based texture
    string fullPath = directory + '/' + filename;

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    // Try to load with a simple check first
    int width, height, nrComponents;
    unsigned char *data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    
    if (data && width > 0 && height > 0)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.0f);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        if (data) stbi_image_free(data);
    }

    return textureID;
}

Mesh::Mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures)
{
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

void Mesh::draw(Shader &shader)
{
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    for(unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);

        // Use the correct uniform name format
        std::string uniformName = name + number;
        shader.setInt(uniformName.c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
  
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);  

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
                 &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texCoords));

    glBindVertexArray(0);
}

Model::Model(const std::string &path)
{
    loadModel(path);
}

void Model::draw(Shader &shader)
{
    draw(shader, glm::mat4(1.0f));
}

void Model::draw(Shader &shader, const glm::mat4& parentTransform, bool renderColliders)
{
    // Render only visual (non-collider) meshes
    for (auto& meshInstance : meshes)
    {
        // Skip collider-only meshes - they're rendered via Bullet's debug drawer
        if (IsColliderMesh(meshInstance.name))
            continue;

        shader.setMat4(
            "model",
            parentTransform * meshInstance.transform
        );

        meshInstance.mesh.draw(shader);
    }
}



btCollisionShape* Model::buildConvexHullCollider()
{
    btConvexHullShape* convexHull = new btConvexHullShape();

    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name))
            continue;

        // Include all meshes in the collision shape
        for (const auto& vertex : meshInstance.mesh.vertices)
        {
            //transfrom vertex position by meshInstance.transform
            glm::vec4 transformedPos = meshInstance.transform * glm::vec4(vertex.position, 1.0f);
            convexHull->addPoint(btVector3(transformedPos.x, transformedPos.y, transformedPos.z));
        }
    }

    convexHull->optimizeConvexHull();// Optional: optimize the hull for better performance
    convexHull->initializePolyhedralFeatures(); // Optional: initialize polyhedral features for better collision detection
    convexHull->recalcLocalAabb(); // Recalculate the bounding box after adding points

    return convexHull;
}
// Builds a compound collider made of boxes, one for each mesh marked as a collider
btCollisionShape* Model::buildCompoundBoxCollider()
{
    btCompoundShape* compound = new btCompoundShape();

    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name))
            continue;

        if (meshInstance.mesh.vertices.empty())
            continue;

        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        // LOCAL mesh bounds only
        for (const auto& vertex : meshInstance.mesh.vertices)
        {
            minBounds = glm::min(minBounds, vertex.position);
            maxBounds = glm::max(maxBounds, vertex.position);
        }

        glm::vec3 center = (minBounds + maxBounds) * 0.5f;
        glm::vec3 halfExtents = (maxBounds - minBounds) * 0.5f;

        halfExtents = glm::max(halfExtents, glm::vec3(0.01f));

        // Extract transform from mesh transform
        glm::mat4 transform = meshInstance.transform;

        // Decompose transform into translation, rotation and scale
        glm::vec3 translation = glm::vec3(transform[3]);
        glm::vec3 col0 = glm::vec3(transform[0]);
        glm::vec3 col1 = glm::vec3(transform[1]);
        glm::vec3 col2 = glm::vec3(transform[2]);

        glm::vec3 scale(
            glm::length(col0),
            glm::length(col1),
            glm::length(col2)
        );

        // Build a pure rotation matrix (remove scale) then extract quaternion
        glm::mat3 rotMat;
        rotMat[0] = (scale.x > 0.0f) ? (col0 / scale.x) : col0;
        rotMat[1] = (scale.y > 0.0f) ? (col1 / scale.y) : col1;
        rotMat[2] = (scale.z > 0.0f) ? (col2 / scale.z) : col2;

        glm::quat rotation = glm::quat_cast(rotMat);

        // Apply absolute scale to the half extents so boxes respect model scaling
        glm::vec3 absScale = glm::abs(scale);
        glm::vec3 scaledHalfExtents = halfExtents * absScale;

        btBoxShape* boxShape = new btBoxShape(
            btVector3(
                scaledHalfExtents.x,
                scaledHalfExtents.y,
                scaledHalfExtents.z
            )
        );

        // Transform center into world (mesh) space
        glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(center, 1.0f));

        btTransform localTransform;
        localTransform.setIdentity();

        localTransform.setOrigin(
            btVector3(worldCenter.x, worldCenter.y, worldCenter.z)
        );

        localTransform.setRotation(
            btQuaternion(
                rotation.x,
                rotation.y,
                rotation.z,
                rotation.w
            )
        );

        compound->addChildShape(localTransform, boxShape);
    }

    return compound;
}

btCollisionShape* Model::buildTriangleMeshCollider()
{
    btTriangleMesh* triangleMesh = new btTriangleMesh();

    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name))
            continue;

        // Include only collider meshes in the collision shape
        const auto& vertices = meshInstance.mesh.vertices;
        const auto& indices = meshInstance.mesh.indices;

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            glm::vec3 v0 =
                glm::vec3(
                    meshInstance.transform *
                    glm::vec4(vertices[indices[i]].position, 1.0f)
                );

            glm::vec3 v1 =
                glm::vec3(
                    meshInstance.transform *
                    glm::vec4(vertices[indices[i + 1]].position, 1.0f)
                );

            glm::vec3 v2 =
                glm::vec3(
                    meshInstance.transform *
                    glm::vec4(vertices[indices[i + 2]].position, 1.0f)
                );

            triangleMesh->addTriangle(
                btVector3(v0.x, v0.y, v0.z),
                btVector3(v1.x, v1.y, v1.z),
                btVector3(v2.x, v2.y, v2.z)
            );
        }
    }

    bool useQuantizedAabbCompression = true;

    return new btBvhTriangleMeshShape(
        triangleMesh,
        useQuantizedAabbCompression
    );
}

btCollisionShape* Model::buildSphericalHullCollider()
{
    btCompoundShape* compound = new btCompoundShape();

    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name))
            continue;

        if (meshInstance.mesh.vertices.empty())
            continue;

        // Calculate bounding box for tighter sphere fit
        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        for (const auto& vertex : meshInstance.mesh.vertices)
        {
            minBounds = glm::min(minBounds, vertex.position);
            maxBounds = glm::max(maxBounds, vertex.position);
        }

        // Sphere center is at the center of the bounding box
        glm::vec3 center = (minBounds + maxBounds) * 0.5f;

        // Sphere radius is half the diagonal of the bounding box
        glm::vec3 halfExtents = (maxBounds - minBounds) * 0.5f;
        float maxRadius = glm::length(halfExtents);

        // Ensure minimum radius for stability
        maxRadius = glm::max(maxRadius, 0.01f);

        btSphereShape* sphereShape = new btSphereShape(maxRadius);

        // Extract transform from mesh transform
        glm::mat4 transform = meshInstance.transform;

        // Apply center offset to position
        glm::vec3 position = glm::vec3(transform * glm::vec4(center, 1.0f));

        glm::quat rotation = glm::quat_cast(transform);

        btTransform localTransform;
        localTransform.setIdentity();

        localTransform.setOrigin(
            btVector3(position.x, position.y, position.z)
        );

        localTransform.setRotation(
            btQuaternion(
                rotation.x,
                rotation.y,
                rotation.z,
                rotation.w
            )
        );

        compound->addChildShape(localTransform, sphereShape);
    }

    return compound;
}

btCollisionShape* Model::buildCapsuleColliderFromMesh()
{
    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);
    bool foundAny = false;

    // 1. Find the bounds of all meshes marked as colliders
    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name)) continue;

        for (const auto& vertex : meshInstance.mesh.vertices)
        {
            // Transform vertex to world space relative to the model
            glm::vec3 worldPos = glm::vec3(meshInstance.transform * glm::vec4(vertex.position, 1.0f));
            minBounds = glm::min(minBounds, worldPos);
            maxBounds = glm::max(maxBounds, worldPos);
            foundAny = true;
        }
    }

    if (!foundAny) return nullptr;

    // 2. Calculate dimensions
    glm::vec3 extents = maxBounds - minBounds;
    float radius = glm::max(extents.x, extents.z) * 0.5f;
    float height = glm::max(0.0f, extents.y - (radius * 2.0f)); // Subtract caps from height

    // 3. Create the capsule
    // Bullet capsules are Y-up by default
    btCapsuleShape* capsule = new btCapsuleShape(radius, height);
    
    return capsule;
}
glm::vec3 ModelLoader::getModelScale(size_t modelIndex) const
{
    if (modelIndex >= models.size())
        return glm::vec3(1.0f);

    const glm::mat4& m = models[modelIndex].transform;

    return glm::vec3(
        glm::length(glm::vec3(m[0])),
        glm::length(glm::vec3(m[1])),
        glm::length(glm::vec3(m[2]))
    );
}

void Model::loadModel(const std::string &path)
{
    Assimp::Importer importer;
    unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | 
                         aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
    const aiScene* scene = importer.ReadFile(path, flags);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    scene_ptr = scene;
    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
    glm::mat4 to;

    to[0][0] = from.a1; to[1][0] = from.a2;
    to[2][0] = from.a3; to[3][0] = from.a4;

    to[0][1] = from.b1; to[1][1] = from.b2;
    to[2][1] = from.b3; to[3][1] = from.b4;

    to[0][2] = from.c1; to[1][2] = from.c2;
    to[2][2] = from.c3; to[3][2] = from.c4;

    to[0][3] = from.d1; to[1][3] = from.d2;
    to[2][3] = from.d3; to[3][3] = from.d4;

    return to;
}

void Model::processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform)
{
    glm::mat4 transform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        MeshInstance instance;
        instance.mesh = processMesh(mesh, scene);
        instance.transform = transform;
        instance.name = node->mName.C_Str();
        meshes.push_back(instance);
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, transform);
    }

}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<texture> textures;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertex vertex;
        glm::vec3 vector; 
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z; 
        vertex.position = vector;

        if(mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        
        if(mesh->mTextureCoords[0]) 
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x; 
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else
            vertex.texCoords = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    // If no textures were loaded, add a default white texture so the model isn't black
    if(textures.empty())
    {
        texture defaultTexture;
        defaultTexture.id = CreateWhiteTexture();
        defaultTexture.type = "texture_diffuse";
        defaultTexture.path = "default_white";
        textures.push_back(defaultTexture);
    }
    

    return Mesh(vertices, indices, textures);
}

std::vector<texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName)
{
    std::vector<texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; 
                break;
            }
        }
        if(!skip)
        {   
            texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory, scene_ptr);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); 
        }
    }
    return textures;
}

void ModelLoader::loadModel(
    const std::string& modelPath,
    const glm::vec3& position,
    const glm::vec3& scale)
{
    try
    {
        std::shared_ptr<Model> modelPtr;

        // Already loaded
        if (modelCache.find(modelPath) != modelCache.end())
        {
            modelPtr = modelCache[modelPath];
        }
        else
        {
            modelPtr = std::make_shared<Model>(modelPath);

            modelCache[modelPath] = modelPtr;
        }

        glm::mat4 modelMatrix = glm::mat4(1.0f);

        modelMatrix =
            glm::translate(modelMatrix, position);

        modelMatrix =
            glm::scale(modelMatrix, scale);

        models.push_back({
            modelPtr,
            modelMatrix
        });
    }
    catch (const std::exception& e)
    {
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

void ModelLoader::setModelTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale, float rotationAngle, const glm::vec3& rotationAxis) {
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

void ModelLoader::updateModelTransform(size_t modelIndex, const glm::vec3& deltaPosition, const glm::vec3& deltaScale, float deltaRotationAngle, const glm::vec3& rotationAxis) {
    if (modelIndex >= models.size()) {
        std::cerr << "Invalid model index: " << modelIndex << std::endl;
        return;
    }

    // Apply incremental transformations
    glm::mat4& modelMatrix = models[modelIndex].transform;
    
    // Apply translation
    modelMatrix = glm::translate(modelMatrix, deltaPosition);
    
    // Apply rotation
    modelMatrix = glm::rotate(modelMatrix, deltaRotationAngle, rotationAxis);
    
    // Apply scaling
    modelMatrix = glm::scale(modelMatrix, deltaScale);
}

glm::vec3 ModelLoader::getModelPosition(size_t modelIndex) const {
    if (modelIndex >= models.size()) {
        std::cerr << "Invalid model index: " << modelIndex << std::endl;
        return glm::vec3(0.0f);
    }
    
    // Extract position from the transformation matrix (last column)
    return glm::vec3(models[modelIndex].transform[3]);
}