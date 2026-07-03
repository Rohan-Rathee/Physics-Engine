#include "model_serializer.h" 

#include <iostream> 

#include <assimp/Importer.hpp> 

#include <assimp/scene.h> 

#include <assimp/postprocess.h> 

  

const uint32_t MODEL_MAGIC = 0x414E4944;   

const uint32_t MODEL_VERSION = 1; 

bool ModelSerializer::serializeToFile(const std::string& modelPath, const std::string& outputPath) { 

    try { 

          

        Assimp::Importer importer; 

        const aiScene* scene = importer.ReadFile(modelPath, 

            aiProcess_Triangulate | 

            aiProcess_CalcTangentSpace | 

            aiProcess_GenNormals | 

            aiProcess_OptimizeMeshes 

        ); 

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { 

            std::cerr << "Failed to load model: " << importer.GetErrorString() << std::endl; 

            return false; 

        } 

          

        std::ofstream outFile(outputPath, std::ios::binary); 

        if (!outFile.is_open()) { 

            std::cerr << "Failed to open output file: " << outputPath << std::endl; 

            return false; 

        } 

          

        outFile.write((const char*)&MODEL_MAGIC, sizeof(MODEL_MAGIC)); 

        outFile.write((const char*)&MODEL_VERSION, sizeof(MODEL_VERSION)); 

          

        uint32_t numMeshes = scene->mNumMeshes; 

        outFile.write((const char*)&numMeshes, sizeof(numMeshes)); 

          

        for (uint32_t i = 0; i < numMeshes; ++i) { 

            aiMesh* mesh = scene->mMeshes[i]; 

              

            uint32_t numVertices = mesh->mNumVertices; 

            uint32_t numFaces = mesh->mNumFaces; 

            uint32_t numIndices = numFaces * 3;   

            outFile.write((const char*)&numVertices, sizeof(numVertices)); 

            outFile.write((const char*)&numIndices, sizeof(numIndices)); 

              

            for (uint32_t v = 0; v < numVertices; ++v) { 

                  

                glm::vec3 pos(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z); 

                outFile.write((const char*)&pos, sizeof(pos)); 

                  

                glm::vec3 normal(0.0f); 

                if (mesh->HasNormals()) { 

                    normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z); 

                } 

                outFile.write((const char*)&normal, sizeof(normal)); 

                  

                glm::vec2 texCoord(0.0f); 

                if (mesh->HasTextureCoords(0)) { 

                    texCoord = glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y); 

                } 

                outFile.write((const char*)&texCoord, sizeof(texCoord)); 

            } 

              

            for (uint32_t f = 0; f < numFaces; ++f) { 

                aiFace& face = mesh->mFaces[f]; 

                for (uint32_t idx = 0; idx < face.mNumIndices; ++idx) { 

                    uint32_t index = face.mIndices[idx]; 

                    outFile.write((const char*)&index, sizeof(index)); 

                } 

            } 

        } 

        outFile.close(); 

        std::cout << "Model serialized successfully to: " << outputPath << std::endl; 

        return true; 

    } catch (const std::exception& e) { 

        std::cerr << "Serialization error: " << e.what() << std::endl; 

        return false; 

    } 

} 

bool ModelSerializer::deserializeFromFile(const std::string& inputPath, 

                                           std::vector<Mesh>& outMeshes, 

                                           std::string& outDirectory) { 

    try { 

        std::ifstream inFile(inputPath, std::ios::binary); 

        if (!inFile.is_open()) { 

            std::cerr << "Failed to open input file: " << inputPath << std::endl; 

            return false; 

        } 

          

        uint32_t magic = 0; 

        uint32_t version = 0; 

        inFile.read((char*)&magic, sizeof(magic)); 

        inFile.read((char*)&version, sizeof(version)); 

        if (magic != MODEL_MAGIC) { 

            std::cerr << "Invalid model file format" << std::endl; 

            return false; 

        } 

        if (version != MODEL_VERSION) { 

            std::cerr << "Unsupported model version: " << version << std::endl; 

            return false; 

        } 

          

        uint32_t numMeshes = 0; 

        inFile.read((char*)&numMeshes, sizeof(numMeshes)); 

          

        size_t lastSlash = inputPath.find_last_of("/\\"); 

        outDirectory = (lastSlash != std::string::npos) ? inputPath.substr(0, lastSlash) : ""; 

        outMeshes.clear(); 

          

        for (uint32_t i = 0; i < numMeshes; ++i) { 

            uint32_t numVertices = 0; 

            uint32_t numIndices = 0; 

            inFile.read((char*)&numVertices, sizeof(numVertices)); 

            inFile.read((char*)&numIndices, sizeof(numIndices)); 

            std::vector<vertex> vertices; 

            std::vector<unsigned int> indices; 

              

            for (uint32_t v = 0; v < numVertices; ++v) { 

                vertex vert; 

                inFile.read((char*)&vert.position, sizeof(vert.position)); 

                inFile.read((char*)&vert.normal, sizeof(vert.normal)); 

                inFile.read((char*)&vert.texCoords, sizeof(vert.texCoords)); 

                vertices.push_back(vert); 

            } 

              

            for (uint32_t idx = 0; idx < numIndices; ++idx) { 

                unsigned int index = 0; 

                inFile.read((char*)&index, sizeof(index)); 

                indices.push_back(index); 

            } 

              

            std::vector<texture> textures; 

            outMeshes.emplace_back(vertices, indices, textures); 

        } 

        inFile.close(); 

        std::cout << "Model deserialized successfully from: " << inputPath << std::endl; 

        return true; 

    } catch (const std::exception& e) { 

        std::cerr << "Deserialization error: " << e.what() << std::endl; 

        return false; 

    } 

} 

