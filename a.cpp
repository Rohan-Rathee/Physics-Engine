#include <iostream>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include "stb_image.h"

struct TerrainVertex {
    glm::vec3 position;
    glm::vec2 texCoord;
    glm::vec3 normal;
};

struct TerrainHeader {
    uint32_t magic;         // "TERR"
    uint32_t vertexCount;
    uint32_t indexCount;
    float    minHeight;
    float    maxHeight;
};

int main() {
    int width  = 5280;
    int height = 3085;
    int scale  = 1;
    std::vector<float> hmap(width * height);

    std::ifstream raw("Textures/terrain.raw", std::ios::binary);
    if (!raw) { std::cerr << "Failed to open terrain.raw" << std::endl; return 1; }
    raw.read((char*)hmap.data(), hmap.size() * sizeof(float));
    raw.close();

    // Find min/max
    float minH =  999999.0f;
    float maxH = -999999.0f;
    for (float h : hmap) {
        if (h < minH) minH = h;
        if (h > maxH) maxH = h;
    }

    std::cout << "Min: " << minH << "m  Max: " << maxH << "m" << std::endl;

    // Build vertices
    std::vector<TerrainVertex> vertices;
    vertices.reserve(width * height);

    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            TerrainVertex v;
            v.position = glm::vec3(x * scale, hmap[z * width + x], z * scale);
            v.texCoord = glm::vec2((float)x / width, (float)z / height);
            v.normal   = glm::vec3(0, 1, 0);
            vertices.push_back(v);
        }
    }

    // Calculate normals
    for (int z = 1; z < height-1; z++) {
        for (int x = 1; x < width-1; x++) {
            float hL = hmap[z*width + (x-1)];
            float hR = hmap[z*width + (x+1)];
            float hD = hmap[(z-1)*width + x];
            float hU = hmap[(z+1)*width + x];
            vertices[z*width + x].normal =
                glm::normalize(glm::vec3((hL-hR)/(2*scale),
                                          1.0f,
                                         (hD-hU)/(2*scale)));
        }
    }

    // Build indices
    std::vector<uint32_t> indices;
    indices.reserve((width-1) * (height-1) * 6);

    for (int z = 0; z < height-1; z++) {
        for (int x = 0; x < width-1; x++) {
            uint32_t tl = z*width + x;
            indices.push_back(tl);
            indices.push_back(tl + width);
            indices.push_back(tl + 1);
            indices.push_back(tl + 1);
            indices.push_back(tl + width);
            indices.push_back(tl + width + 1);
        }
    }

    // Write binary
    std::ofstream file("Textures/terrain.terrainbin", std::ios::binary);

    TerrainHeader header;
    header.magic       = 0x54455252;
    header.vertexCount = vertices.size();
    header.indexCount  = indices.size();
    header.minHeight   = minH;
    header.maxHeight   = maxH;

    file.write((char*)&header,         sizeof(TerrainHeader));
    file.write((char*)vertices.data(), vertices.size() * sizeof(TerrainVertex));
    file.write((char*)indices.data(),  indices.size()  * sizeof(uint32_t));
    file.close();

    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Indices:  " << indices.size()  << std::endl;
    std::cout << "Done -> terrain.terrainbin" << std::endl;
    return 0;
}