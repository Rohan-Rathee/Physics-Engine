#ifndef MODEL_SERIALIZER_H
#define MODEL_SERIALIZER_H

#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <glm/glm.hpp>
#include "model_loader.h"

class ModelSerializer {
public:
    // Serialize a Model to a binary file
    static bool serializeToFile(const std::string& modelPath, const std::string& outputPath);
    
    // Deserialize a Model from a binary file
    static bool deserializeFromFile(const std::string& inputPath, 
                                     std::vector<Mesh>& outMeshes,
                                     std::string& outDirectory);

private:
    // Helper: Write vertex data to stream
    static void writeVertex(std::ostream& stream, const vertex& v);
    static void readVertex(std::istream& stream, vertex& v);
    
    // Helper: Write mesh data to stream
    static void writeMesh(std::ostream& stream, const Mesh& mesh);
    static Mesh readMesh(std::istream& stream);
};

#endif
