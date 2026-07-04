/**
 * @file render_system.h
 * @brief Central rendering system for the engine.
 *
 * Responsible for the entire rendering pipeline, including:
 * - Model rendering.
 * - Physically Based Rendering (PBR). yea just a bunch of formulae in shader, but was really intresting once implements, even emissions work now 
 * - HDR rendering and tonemapping.
 * - Bloom post-processing. it was hell.
 * - Shadow mapping. only for the first directional light, but can be extended to multiple lights if needed but destroyes performance on high res shadows
 * - Image Based Lighting (IBL).
 * - Skybox rendering. removed. reimplemented with blank hdri
 * - Frustum culling. removed. preformance not priority for this project, and it was causing issues with model rendering and shadows, will reimplement later if needed
 * - Bullet physics debug rendering. !!reduces fps and causes physics issues.
 *
 * Acts as the bridge between engine data and the GPU by managing
 * rendering resources, shaders, framebuffers, and draw passes.
 *
 * ------------------------
 * This is one of the biggest files in the engine. i admit it handles too much, even imports the models and owns the physics body 
 * ------------------------
 */
#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../camera.h"
#include "../shader.h"
#include "../utils/bullet_debug_drawer.h"
#include "../utils/model_loader.h"
#include "../utils/model_transform.h"
#include <memory>
#include <string>
#include "../core/time_manager.h"
class btDynamicsWorld;
class LightManager;
class RenderSystem {
private:
    unsigned int hdrTexture = 0;
    unsigned int envCubemap = 0;
    unsigned int irradianceMap = 0;
    unsigned int prefilterMap = 0;
    unsigned int brdfLUTTexture = 0;
    unsigned int captureFBO = 0;
    unsigned int captureRBO = 0;
    unsigned int skyboxVAO = 0;
    unsigned int skyboxVBO = 0;
    unsigned int brdfQuadVAO = 0;
    unsigned int brdfQuadVBO = 0;
    bool hdriLoaded = false;
    float hdriExposure = 1.0f;
    std::unique_ptr<Shader> equirectShader;
    std::unique_ptr<Shader> irradianceShader;
    std::unique_ptr<Shader> prefilterShader;
    std::unique_ptr<Shader> brdfShader;
    std::unique_ptr<Shader> skyboxShader;
    void setupHDRI(const std::string &hdrPath);
    void renderSkybox(const glm::mat4 &view, const glm::mat4 &projection);
    void renderCubeForCapture();
    void renderQuadForCapture();
    unsigned int createCubemap(int size, GLenum internalFormat, bool mipmap = false);
    unsigned int hdrFBO = 0;
    unsigned int hdrColorBuffer = 0;
    unsigned int brightColorBuffer = 0;
    unsigned int hdrRBO = 0;
    unsigned int pingpongFBO[2] = {0, 0};
    unsigned int pingpongBuffer[2] = {0, 0};
    int bloomIterations = 10;
    std::unique_ptr<Shader> bloomBlurShader;
    std::unique_ptr<Shader> compositeShader;
    void setupBloomFBO();
    void BloomPass();
    void CompositePass();
    std::unique_ptr<Shader> modelShader;
    std::unique_ptr<Shader> lightingShader;
    std::unique_ptr<ModelLoader> modelLoader;
    ModelTransform *modelTransform = nullptr;
    unsigned int instanceVBO = 0;
    unsigned int VAO = 0, VBO = 0, lightVAO = 0;
    unsigned int texture1 = 0, texture2 = 0;
    unsigned int screenWidth, screenHeight;
    std::string vertexPath, fragmentPath;
    std::string modelVertexPath, modelFragmentPath;
    std::vector<glm::mat4> instanceMatrices;
    glm::vec4 frustumPlanes[6];
    unsigned int shadowFBO = 0, shadowDepthMap = 0;
    static constexpr unsigned int SHADOW_WIDTH = 10240;
    static constexpr unsigned int SHADOW_HEIGHT = 10240;
    std::unique_ptr<Shader> shadowShader;
    glm::mat4 lightProjection, lightView;
    glm::vec3 lightPos;
    glm::vec3 lightDir;
    std::vector<glm::vec3> cubePositions;
    void setupModels();
    void loadEnvironmentFolder(); // removed cuz i needed simplification
    void setupShadowFramebuffer();
    void ShadowPass(float currentFrame, const glm::mat4 lightSpaceMatrix, const glm::vec3 &cameraPos);
    void RenderPass(const Camera &camera, const glm::mat4 &view, const glm::mat4 &projection, glm::mat4 lightSpaceMatrix, float currentFrame);
    float exposure = 1.0f;
    float bloomStrength = 0.04f;
public:
    LightManager *m_lightManager = nullptr;
    void setLightManager(LightManager *lm) { m_lightManager = lm; }
    void setExposure(float exp) { exposure = glm::max(0.1f, exp); }
    void setBloomStrength(float str) { bloomStrength = glm::max(0.0f, str); }
    void setHDRIExposure(float exp) { hdriExposure = glm::max(0.1f, exp); }
    float getExposure() const { return exposure; }
    float getBloomStrength() const { return bloomStrength; }
    float getHDRIExposure() const { return hdriExposure; }
    RenderSystem(const std::string &vertexPath, const std::string &fragmentPath, unsigned int width, unsigned int height);
    ~RenderSystem();
    bool initialize();
    bool initializeModels();
    void render(const Camera &camera, float currentFrame, const glm::mat4 &view, const glm::mat4 &projection);
    void setScreenSize(unsigned int w, unsigned int h) {
        screenWidth = w;
        screenHeight = h;
    }
    void extractFrustumPlanes(const glm::mat4 &vp);
    bool isInFrustum(const glm::vec3 &pos);
    void resizeBloomBuffers(int width, int height);
    void loadModel(const std::string &modelPath, const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &scale = glm::vec3(1.0f)) {
        modelLoader->loadModel(modelPath, position, scale);
    }
    void clearModels() { modelLoader->clearModels(); }
    void setModelTransform(size_t modelIndex, const glm::vec3 &position, const glm::vec3 &scale, float rotationAngle = 0.0f, const glm::vec3 &rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f)) {
        if (modelTransform) {
            modelTransform->setTransform(modelIndex, position, scale, rotationAngle, rotationAxis);
        }
    }
    ModelLoader *getModelLoader() { return modelLoader.get(); }
    void setModelTransformPtr(ModelTransform *mt) { modelTransform = mt; }
    void setPhysicsWorldPtr(btDynamicsWorld *world) { physicsWorld = world; }
    void setBulletDebugDrawEnabled(bool enabled) { bulletDebugDrawEnabled = enabled; }
    std::unique_ptr<Shader> debugShader;
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;
    std::unique_ptr<BulletDebugDrawer> bulletDebugDrawer;
    btDynamicsWorld *physicsWorld = nullptr;
    std::unique_ptr<Shader> debugLineShader;
    bool bulletDebugDrawEnabled = false;
    void setupDebugQuad();
    void renderShadowMapDebug();
};
#endif