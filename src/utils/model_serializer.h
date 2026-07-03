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

      

    static bool serializeToFile(const std::string& modelPath, const std::string& outputPath); 

     

      

    static bool deserializeFromFile(const std::string& inputPath,  

                                     std::vector<Mesh>& outMeshes, 

                                     std::string& outDirectory); 

private: 

      

    static void writeVertex(std::ostream& stream, const vertex& v); 

    static void readVertex(std::istream& stream, vertex& v); 

     

      

    static void writeMesh(std::ostream& stream, const Mesh& mesh); 

    static Mesh readMesh(std::istream& stream); 

}; 

#endif 

