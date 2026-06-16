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

unsigned int CreateWhiteTexture()
{
    unsigned char whitePixel[] = {255, 255, 255, 255};
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    return textureID;
}

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

unsigned int TextureFromFile(const char *path, const string &directory, const aiScene* scene = nullptr)
{
    string filename = string(path);
    

    if (filename[0] == '*')
    {
        if (!scene)
        {
            std::cerr << "Embedded texture reference but no scene provided: " << filename << std::endl;
            return 0;
        }
        

        int textureIndex = std::atoi(filename.c_str() + 1);
        if (textureIndex < 0 || textureIndex >= static_cast<int>(scene->mNumTextures))
        {
            std::cerr << "Invalid embedded texture index: " << textureIndex << std::endl;
            return 0;
        }
        
        aiTexture* embeddedTexture = scene->mTextures[textureIndex];
        
        if (embeddedTexture->mHeight == 0)
        {

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

            unsigned int textureID = TextureFromEmbeddedData(
                reinterpret_cast<aiTexel*>(embeddedTexture->pcData),
                embeddedTexture->mWidth,
                embeddedTexture->mHeight
            );
            return textureID;
        }
    }
    

    string fullPath = directory + '/' + filename;

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);


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
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;
        if(name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if(name == "texture_specular")
            number = std::to_string(specularNr++);


        std::string uniformName = name + number;
        shader.setInt(uniformName.c_str(), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);


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


    glEnableVertexAttribArray(0);	
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)0);

    glEnableVertexAttribArray(1);	
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, normal));

    glEnableVertexAttribArray(2);	
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, texCoords));


    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(vertex), (void*)offsetof(vertex, boneIDs));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, weights));
    glBindVertexArray(0);

}

void SetVertexBoneData(vertex& vert, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (vert.boneIDs[i] == -1)
        {
            vert.boneIDs[i] = boneID;
            vert.weights[i] = weight;
            return;
        }
    }

    std::cerr << "Warning: More than " << MAX_BONE_INFLUENCE << " bones influencing a vertex. Extra influences will be ignored." << std::endl;
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

    for (auto& meshInstance : meshes)
    {

        if (IsColliderMesh(meshInstance.name))
            continue;

        shader.setMat4(
            "model",
            parentTransform * meshInstance.transform
        );

        meshInstance.mesh.draw(shader);
    }
}


void ModelLoader::blendModelAnimations(
    size_t modelIndex,
    const std::vector<std::pair<unsigned int, float>>& layers)
{

    if (modelIndex >= models.size())
    {
        std::cerr << "ModelLoader::blendModelAnimations: invalid model index "
                  << modelIndex << std::endl;
        return;
    }
 
    if (layers.empty())
    {
        std::cerr << "ModelLoader::blendModelAnimations: layer list is empty, "
                     "ignoring call." << std::endl;
        return;
    }
 
    ModelData&    data  = models[modelIndex];
    const aiScene* scene = data.model->getScene();
 
    if (!scene || !scene->HasAnimations())
    {
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
 
    for (const auto& [animIndex, weight] : layers)
    {
        if (animIndex >= scene->mNumAnimations)
        {
            std::cerr << "ModelLoader::blendModelAnimations: animation index "
                      << animIndex << " out of range (model has "
                      << scene->mNumAnimations << " animations). "
                         "Skipping this layer." << std::endl;
            continue;
        }
 

        data.blendAnimations.push_back(
            std::make_unique<Animation>(scene, data.model.get(), animIndex)
        );
 
        BlendLayer layer;
        layer.animation = data.blendAnimations.back().get();
        layer.weight    = weight;
        layer.time      = 0.0f;
 
        animatorLayers.push_back(layer);
    }
 
    if (animatorLayers.empty())
    {
        std::cerr << "ModelLoader::blendModelAnimations: no valid layers "
                     "could be built." << std::endl;
        return;
    }
 


    data.animator->SetBlendLayers(std::move(animatorLayers));
 


    data.animation.reset(
        new Animation(scene, data.model.get(),
                      layers[0].first)
    );
}

void ModelLoader::setBlendWeights(size_t modelIndex, const std::vector<float>& weights)
{
    if (modelIndex >= models.size() || !models[modelIndex].animator)
        return;
 
    Animator& animator = *models[modelIndex].animator;
    for (size_t i = 0; i < weights.size(); i++)
        animator.SetLayerWeight(i, weights[i]);
}
 

void Model::draw(Shader &shader, const glm::mat4& parentTransform, bool renderColliders, bool skipMeshTransform)
{
    for (auto& meshInstance : meshes)
    {
        if (IsColliderMesh(meshInstance.name))
            continue;

        glm::mat4 modelMatrix = skipMeshTransform
            ? parentTransform
            : parentTransform * meshInstance.transform;

        shader.setMat4("model", modelMatrix);
        meshInstance.mesh.draw(shader);
    }
}
void Model::draw(Shader& shader, const glm::mat4& parentTransform, const std::vector<glm::mat4>& boneMatrices, bool renderColliders)

{
    bool isAnimated = !boneMatrices.empty();

    if (isAnimated)
    {
        shader.setBool("hasAnimation", true);
        for (size_t i = 0; i < boneMatrices.size(); i++)
            shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", boneMatrices[i]);
    }
    else
    {
        shader.setBool("hasAnimation", false);
    }

    draw(shader, parentTransform, renderColliders, isAnimated);
}

btCollisionShape* Model::buildConvexHullCollider()
{
    btConvexHullShape* convexHull = new btConvexHullShape();

    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name))
            continue;


        for (const auto& vertex : meshInstance.mesh.vertices)
        {

            glm::vec4 transformedPos = meshInstance.transform * glm::vec4(vertex.position, 1.0f);
            convexHull->addPoint(btVector3(transformedPos.x, transformedPos.y, transformedPos.z));
        }
    }

    convexHull->optimizeConvexHull();
    convexHull->initializePolyhedralFeatures();
    convexHull->recalcLocalAabb();

    return convexHull;
}

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


        for (const auto& vertex : meshInstance.mesh.vertices)
        {
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
            glm::length(col2)
        );


        glm::mat3 rotMat;
        rotMat[0] = (scale.x > 0.0f) ? (col0 / scale.x) : col0;
        rotMat[1] = (scale.y > 0.0f) ? (col1 / scale.y) : col1;
        rotMat[2] = (scale.z > 0.0f) ? (col2 / scale.z) : col2;

        glm::quat rotation = glm::quat_cast(rotMat);


        glm::vec3 absScale = glm::abs(scale);
        glm::vec3 scaledHalfExtents = halfExtents * absScale;

        btBoxShape* boxShape = new btBoxShape(
            btVector3(
                scaledHalfExtents.x,
                scaledHalfExtents.y,
                scaledHalfExtents.z
            )
        );


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


        glm::vec3 minBounds(FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        for (const auto& vertex : meshInstance.mesh.vertices)
        {
            minBounds = glm::min(minBounds, vertex.position);
            maxBounds = glm::max(maxBounds, vertex.position);
        }


        glm::vec3 center = (minBounds + maxBounds) * 0.5f;


        glm::vec3 halfExtents = (maxBounds - minBounds) * 0.5f;
        float maxRadius = glm::length(halfExtents);


        maxRadius = glm::max(maxRadius, 0.01f);

        btSphereShape* sphereShape = new btSphereShape(maxRadius);


        glm::mat4 transform = meshInstance.transform;


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


    for (const auto& meshInstance : meshes)
    {
        if (!IsColliderMesh(meshInstance.name)) continue;

        for (const auto& vertex : meshInstance.mesh.vertices)
        {

            glm::vec3 worldPos = glm::vec3(meshInstance.transform * glm::vec4(vertex.position, 1.0f));
            minBounds = glm::min(minBounds, worldPos);
            maxBounds = glm::max(maxBounds, worldPos);
            foundAny = true;
        }
    }

    if (!foundAny) return nullptr;


    glm::vec3 extents = maxBounds - minBounds;
    float radius = glm::max(extents.x, extents.z) * 0.5f;
    float height = glm::max(0.0f, extents.y - (radius * 2.0f));



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
    importer = std::make_unique<Assimp::Importer>();
    unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | 
                         aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
    const aiScene* scene = importer->ReadFile(path, flags);
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer->GetErrorString() << std::endl;
        return;
    }
    scene_ptr = scene;
    
    if (scene->HasAnimations()){
        std::cout << "Animations: "
                << scene->mNumAnimations
                << std::endl;
    }


    directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
}

glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
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


void Model::ExtractBoneWeights(std::vector<vertex>& vertices, aiMesh* mesh)
{
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone* bone = mesh->mBones[i];
        std::string boneName(bone->mName.C_Str());


        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            BoneInfo boneInfo;
            boneInfo.id = boneCounter++;
            boneInfo.offset = aiMatrix4x4ToGlm(bone->mOffsetMatrix);
            boneInfoMap[boneName] = boneInfo;
        }

        int boneID = boneInfoMap[boneName].id;

        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
            unsigned int vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;

            SetVertexBoneData(vertices[vertexID], boneID, weight);
        }
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


    if(textures.empty())
    {
        texture defaultTexture;
        defaultTexture.id = CreateWhiteTexture();
        defaultTexture.type = "texture_diffuse";
        defaultTexture.path = "default_white";
        textures.push_back(defaultTexture);
    }
    
    ExtractBoneWeights(vertices, mesh);
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

void ModelLoader::loadModel(    const std::string& modelPath,    const glm::vec3& position,   const glm::vec3& scale){
    try
    {
        std::shared_ptr<Model> modelPtr;


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


    glm::mat4& modelMatrix = models[modelIndex].transform;
    

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

void ModelLoader::setModelAnimation(size_t modelIndex, unsigned int animationIndex)
{
    if (modelIndex >= models.size())
    {
        std::cerr << "ModelLoader::setModelAnimation: invalid model index " << modelIndex << std::endl;
        return;
    }


    ModelData& data = models[modelIndex];
    const aiScene* scene = data.model->getScene();

    if (!scene || !scene->HasAnimations() || animationIndex >= scene->mNumAnimations)
    {
        std::cerr << "ModelLoader::setModelAnimation: model at index " << modelIndex
                  << " has no animation at index " << animationIndex << std::endl;
        return;
    }

    data.animation = std::make_unique<Animation>(scene, data.model.get(), animationIndex);
    data.animator = std::make_unique<Animator>();
    data.animator->PlayAnimation(data.animation.get());
}

void ModelLoader::updateAnimations(float deltaTime)
{
    for (auto& data : models)
    {
        if (data.animator)
            data.animator->UpdateAnimation(deltaTime);
    }
}

bool ModelLoader::hasAnimation(size_t modelIndex) const
{
    return modelIndex < models.size() && models[modelIndex].animator != nullptr;
}

const std::vector<glm::mat4>& ModelLoader::getBoneMatrices(size_t modelIndex) const
{
    static const std::vector<glm::mat4> empty;
    if (modelIndex < models.size() && models[modelIndex].animator)
        return models[modelIndex].animator->GetFinalBoneMatrices();
    return empty;
}