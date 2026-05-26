#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../shader.h"
#include "../utils/model_loader.h"
#include "../camera.h"
#include <string>
#include <memory>
#include "../core/time_manager.h"


class RenderSystem {
private:
    std::unique_ptr<Shader> modelShader;
    std::unique_ptr<Shader> lightingShader;
    std::unique_ptr<ModelLoader> modelLoader;
    unsigned int instanceVBO;
    unsigned int VAO, VBO, lightVAO;
    unsigned int texture1, texture2;
    unsigned int screenWidth, screenHeight;
    std::string vertexPath, fragmentPath;
    std::string modelVertexPath, modelFragmentPath;
    std::vector<glm::mat4> instanceMatrices;
    glm::vec4 frustumPlanes[6];
    unsigned int shadowFBO, shadowDepthMap;
    static constexpr unsigned int SHADOW_WIDTH = 10240;
    static constexpr unsigned int SHADOW_HEIGHT = 10240;

    std::unique_ptr<Shader> shadowShader;
    glm::mat4 lightProjection, lightView;
    glm::vec3 lightPos;
    std::vector<glm::vec3> cubePositions;
    glm::vec3 lightDir;


    void setupModels();
    void setupCube();
    void setupShadowFramebuffer();
    void ShadowPass(float currentFrame, const glm::mat4 lightSpaceMatrix);
    void RenderPass(const Camera& camera, const glm::mat4& view, const glm::mat4& projection, glm::mat4 lightSpaceMatrix, float currentFrame);


public:
    RenderSystem(const std::string& vertexPath, const std::string& fragmentPath, 
                 unsigned int width, unsigned int height);
    ~RenderSystem();

    bool initialize();
    void render(const Camera& camera, float currentFrame, const glm::mat4& view, const glm::mat4& projection);
    void setScreenSize(unsigned int w, unsigned int h) { screenWidth = w; screenHeight = h; }
    void extractFrustumPlanes(const glm::mat4& vp);
    bool isInFrustum(const glm::vec3& pos);
    

    void loadModel(const std::string& modelPath, const glm::vec3& position = glm::vec3(0.0f), 
                   const glm::vec3& scale = glm::vec3(1.0f)) {
        modelLoader->loadModel(modelPath, position, scale);
    }
    void clearModels() { modelLoader->clearModels(); }
    void setModelTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale,
                           float rotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f)) {
        modelLoader->setModelTransform(modelIndex, position, scale, rotationAngle, rotationAxis);
    }

    std::unique_ptr<Shader> debugShader;

    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

    void setupDebugQuad();
    void renderShadowMapDebug();
};


#endif