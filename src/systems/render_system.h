#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../shader.h"
#include "../utils/model_loader.h"
#include <string>
#include <memory>

class RenderSystem {
private:
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> modelShader;
    std::unique_ptr<Shader> lightingShader;
    std::vector<std::pair<std::unique_ptr<Model>, glm::mat4>> models;  // Models with their transforms
    unsigned int instanceVBO;
    unsigned int VAO, VBO, lightVAO;
    unsigned int texture1, texture2;
    unsigned int screenWidth, screenHeight;
    std::string vertexPath, fragmentPath;
    std::string modelVertexPath, modelFragmentPath;
    std::vector<glm::mat4> instanceMatrices;
    glm::vec4 frustumPlanes[6];

public:
    RenderSystem(const std::string& vertexPath, const std::string& fragmentPath, 
                 unsigned int width, unsigned int height);
    ~RenderSystem();

    bool initialize();
    void render(float currentFrame, const glm::mat4& view, const glm::mat4& projection);
    void setScreenSize(unsigned int w, unsigned int h) { screenWidth = w; screenHeight = h; }
    void extractFrustumPlanes(const glm::mat4& vp);
    bool isInFrustum(const glm::vec3& pos);
    
    // Model management methods
    void loadModel(const std::string& modelPath, const glm::vec3& position = glm::vec3(0.0f), 
                   const glm::vec3& scale = glm::vec3(1.0f));
    void clearModels();
    void setModelTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale, 
                          float rotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
};


#endif