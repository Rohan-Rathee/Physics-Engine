#include "render_system.h" 

#include <iostream> 

#define STB_IMAGE_IMPLEMENTATION 

#include "stb_image.h" 

#include <filesystem> 

#include "../utils/light_manager.h"

namespace fs = std::filesystem; 

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); 

static const float kSkyboxVertices[] = { 

    -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

     1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

    -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,

     1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,

     1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,

     1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,

     1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f

}; 

static const glm::mat4 kCaptureViews[6] = { 

    glm::lookAt(glm::vec3(0), glm::vec3( 1, 0, 0), glm::vec3(0,-1, 0)), 

    glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0,-1, 0)), 

    glm::lookAt(glm::vec3(0), glm::vec3( 0, 1, 0), glm::vec3(0, 0, 1)), 

    glm::lookAt(glm::vec3(0), glm::vec3( 0,-1, 0), glm::vec3(0, 0,-1)), 

    glm::lookAt(glm::vec3(0), glm::vec3( 0, 0, 1), glm::vec3(0,-1, 0)), 

    glm::lookAt(glm::vec3(0), glm::vec3( 0, 0,-1), glm::vec3(0,-1, 0)), 

}; 

static const glm::mat4 kCaptureProjection =

    glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f); 

static const float kQuadVerts[] = { 

    -1.0f,  1.0f, 0.0f, 1.0f, 

    -1.0f, -1.0f, 0.0f, 0.0f, 

     1.0f, -1.0f, 1.0f, 0.0f, 

    -1.0f,  1.0f, 0.0f, 1.0f, 

     1.0f, -1.0f, 1.0f, 0.0f, 

     1.0f,  1.0f, 1.0f, 1.0f 

}; 

RenderSystem::RenderSystem(const std::string &vertexPath,

                           const std::string &fragmentPath,

                           unsigned int width, unsigned int height) 

    : modelShader(nullptr),

      modelLoader(std::make_unique<ModelLoader>()),

      modelTransform(nullptr),

      VAO(0), VBO(0),

      texture1(0),

      screenWidth(width), screenHeight(height), 

      vertexPath(vertexPath), fragmentPath(fragmentPath), 

      modelVertexPath("Shaders/model_vertex.glsl"),

      modelFragmentPath("Shaders/model_fragment.glsl"), 

      shadowFBO(0), shadowDepthMap(0), shadowShader(nullptr), 

      bulletDebugDrawer(std::make_unique<BulletDebugDrawer>()),

      physicsWorld(nullptr) 

{} 

RenderSystem::~RenderSystem() 

{ 

    glDeleteVertexArrays(1, &VAO); 

    glDeleteBuffers(1, &VBO); 

    glDeleteFramebuffers(1, &shadowFBO); 

    glDeleteTextures(1, &shadowDepthMap); 

    glDeleteFramebuffers(1,  &hdrFBO);

    glDeleteTextures(1,      &hdrColorBuffer);

    glDeleteTextures(1,      &brightColorBuffer);

    glDeleteRenderbuffers(1, &hdrRBO);

    glDeleteFramebuffers(2,  pingpongFBO);

    glDeleteTextures(2,      pingpongBuffer);

    if (skyboxVAO)    glDeleteVertexArrays(1, &skyboxVAO); 

    if (skyboxVBO)    glDeleteBuffers(1, &skyboxVBO); 

    if (brdfQuadVAO)  glDeleteVertexArrays(1, &brdfQuadVAO); 

    if (brdfQuadVBO)  glDeleteBuffers(1, &brdfQuadVBO); 

    if (captureFBO)   glDeleteFramebuffers(1, &captureFBO); 

    if (captureRBO)   glDeleteRenderbuffers(1, &captureRBO); 

    if (hdrTexture)   glDeleteTextures(1, &hdrTexture); 

    if (envCubemap)   glDeleteTextures(1, &envCubemap); 

    if (irradianceMap)glDeleteTextures(1, &irradianceMap); 

    if (prefilterMap) glDeleteTextures(1, &prefilterMap); 

    if (brdfLUTTexture) glDeleteTextures(1, &brdfLUTTexture); 

} 

unsigned int RenderSystem::createCubemap(int size, GLenum internalFormat, bool mipmap) 

{ 

    unsigned int id; 

    glGenTextures(1, &id); 

    glBindTexture(GL_TEXTURE_CUBE_MAP, id); 

    GLenum fmt  = (internalFormat == GL_RG16F) ? GL_RG  : GL_RGB; 

    GLenum type = (internalFormat == GL_RGB16F || internalFormat == GL_RG16F)

                      ? GL_FLOAT : GL_UNSIGNED_BYTE; 

    for (int face = 0; face < 6; ++face) 

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 

                     internalFormat, size, size, 0, fmt, type, nullptr); 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, 

                    mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR); 

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    if (mipmap) glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 

    return id; 

} 

void RenderSystem::renderCubeForCapture() 

{ 

    glBindVertexArray(skyboxVAO); 

    glDrawArrays(GL_TRIANGLES, 0, 36); 

    glBindVertexArray(0); 

} 

void RenderSystem::renderQuadForCapture() 

{ 

    glBindVertexArray(brdfQuadVAO); 

    glDrawArrays(GL_TRIANGLES, 0, 6); 

    glBindVertexArray(0); 

} 

void RenderSystem::setupBloomFBO()

{

    glGenFramebuffers(1, &hdrFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(1, &hdrColorBuffer);

    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,

                 screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,

                           GL_TEXTURE_2D, hdrColorBuffer, 0);

    glGenTextures(1, &brightColorBuffer);

    glBindTexture(GL_TEXTURE_2D, brightColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,

                 screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,

                           GL_TEXTURE_2D, brightColorBuffer, 0);

    glGenRenderbuffers(1, &hdrRBO);

    glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,

                          screenWidth, screenHeight);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,

                              GL_RENDERBUFFER, hdrRBO);

    unsigned int hdrAttachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

    glDrawBuffers(2, hdrAttachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)

        std::cerr << "[Bloom] HDR FBO is not complete!" << std::endl;

    glGenFramebuffers(2,  pingpongFBO);

    glGenTextures(2,      pingpongBuffer);

    for (int i = 0; i < 2; ++i)

    {

        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);

        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,

                     screenWidth, screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,

                               GL_TEXTURE_2D, pingpongBuffer[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)

            std::cerr << "[Bloom] Ping-pong FBO " << i << " is not complete!" << std::endl;

    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "[Bloom] HDR + ping-pong FBOs created ("

              << screenWidth << "x" << screenHeight << ")." << std::endl;

}

void RenderSystem::BloomPass()

{

    if (!bloomBlurShader) return;

    bool horizontal = true;

    bool firstIteration = true;

    bloomBlurShader->use();

    bloomBlurShader->setInt("image", 0);

    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < bloomIterations; ++i)

    {

        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal ? 1 : 0]);

        bloomBlurShader->setBool("horizontal", horizontal);

        

        

        glBindTexture(GL_TEXTURE_2D,

                      firstIteration ? brightColorBuffer

                                     : pingpongBuffer[horizontal ? 0 : 1]);

 

        renderQuadForCapture();

        horizontal   = !horizontal;

        firstIteration = false;

    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void RenderSystem::CompositePass()

{

    if (!compositeShader) return;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);

    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);   

    compositeShader->use();

    compositeShader->setInt("hdrScene",    0);

    compositeShader->setInt("bloomBlur",   1);

    compositeShader->setFloat("bloomStrength", bloomStrength);

    compositeShader->setFloat("exposure",      exposure);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

    glActiveTexture(GL_TEXTURE1);

    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[0]);

    renderQuadForCapture();

    glEnable(GL_DEPTH_TEST);

}

void RenderSystem::setupModels() 

{ 

    btCollisionShape *basketballShape = nullptr; 

    for (int i = 0; i < 1; ++i) 

    { 

        for (int j = 0; j < 1; ++j) 

        { 

            loadModel("models\\p1.glb", glm::vec3(0.0f), glm::vec3(1.0f)); 

            int currentIndex = (int)modelLoader->models.size() - 1; 

            modelLoader->setModelAnimation(currentIndex, 0); 

            if (basketballShape == nullptr && modelTransform) 

            { 

                Model *m = modelLoader->models[currentIndex].model.get(); 

                basketballShape = m->buildCapsuleColliderFromMesh(); 

                glm::vec3 scale = modelLoader->getModelScale(currentIndex);   

                basketballShape->setLocalScaling(btVector3(scale.x, scale.y, scale.z)); 

            } 

            setModelTransform(currentIndex, glm::vec3(0.0f, 10.0f, 0.0f),

                              glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)); 

            if (modelTransform && basketballShape) 

                modelTransform->initializePhysicsBody(currentIndex, 80.0f,

                                                      basketballShape, 0.1f); 

        } 

    } 

    loadModel("models\\parts1.glb", glm::vec3(0.0f), glm::vec3(1.0f));

    modelTransform->initializePhysicsBody(

        modelLoader->models.size() - 1, 0.0f,

        modelLoader->models.back().model->buildCompoundBoxCollider(), 0.2f);

}   

void RenderSystem::setupShadowFramebuffer() 

{ 

    shadowShader = std::make_unique<Shader>("Shaders/shadow_vertex.glsl",

                                            "Shaders/shadow_fragment.glsl"); 

    glGenFramebuffers(1, &shadowFBO); 

    glGenTextures(1, &shadowDepthMap); 

    glBindTexture(GL_TEXTURE_2D, shadowDepthMap); 

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,

                 SHADOW_WIDTH, SHADOW_HEIGHT, 0,

                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f}; 

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO); 

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,

                           GL_TEXTURE_2D, shadowDepthMap, 0); 

    glDrawBuffer(GL_NONE); 

    glReadBuffer(GL_NONE); 

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 

        std::cerr << "Shadow framebuffer is not complete!" << std::endl; 

    lightPos = glm::vec3(10.0f, 10.0f, 10.0f); 

    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f); 

    lightView       = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f)); 

    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 200.0f); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

} 

bool RenderSystem::initialize() 

{ 

    glEnable(GL_DEPTH_TEST); 

    glDepthFunc(GL_LEQUAL); 

    glEnable(GL_CULL_FACE); 

    glCullFace(GL_BACK); 

    glEnable(GL_MULTISAMPLE); 

    glEnable(GL_BLEND); 

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    modelShader = std::make_unique<Shader>(modelVertexPath.c_str(),

                                           modelFragmentPath.c_str()); 

    setupShadowFramebuffer(); 

    setupDebugQuad(); 

    bulletDebugDrawer->initBuffers(); 

    debugLineShader = std::make_unique<Shader>("Shaders/debug_line_vertex.glsl",

                                               "Shaders/debug_line_fragment.glsl"); 

    setupHDRI("models/cobblestone_parish_road_4k.hdr"); 

    bloomBlurShader = std::make_unique<Shader>("Shaders/brdf_vertex.glsl",

                                               "Shaders/bloom_blur_fragment.glsl");

    compositeShader = std::make_unique<Shader>("Shaders/brdf_vertex.glsl",

                                               "Shaders/composite_fragment.glsl");

    setupBloomFBO();

    return true; 

} 

bool RenderSystem::initializeModels() 

{ 

    if (!modelTransform) 

    { 

        std::cerr << "RenderSystem::initializeModels: modelTransform not set!" << std::endl; 

        return false; 

    } 

    setupModels(); 

    return true; 

} 

void RenderSystem::setupHDRI(const std::string &hdrPath) 

{ 

    glGenVertexArrays(1, &skyboxVAO); 

    glGenBuffers(1, &skyboxVBO); 

    glBindVertexArray(skyboxVAO); 

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO); 

    glBufferData(GL_ARRAY_BUFFER, sizeof(kSkyboxVertices), kSkyboxVertices, GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); 

    glBindVertexArray(0); 

    stbi_set_flip_vertically_on_load(true); 

    int w, h, channels; 

    float *data = stbi_loadf(hdrPath.c_str(), &w, &h, &channels, 0); 

    if (!data) { std::cerr << "[HDRI] Failed to load: " << hdrPath << std::endl; return; } 

    glGenTextures(1, &hdrTexture); 

    glBindTexture(GL_TEXTURE_2D, hdrTexture); 

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, data); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    stbi_image_free(data); 

    std::cout << "[HDRI] Loaded " << hdrPath << " (" << w << "x" << h << ")" << std::endl; 

    glGenFramebuffers(1,  &captureFBO); 

    glGenRenderbuffers(1, &captureRBO); 

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); 

    equirectShader = std::make_unique<Shader>("Shaders/cubemap_capture_vertex.glsl",

                                              "Shaders/equirect_to_cubemap_fragment.glsl"); 

    const int ENV_SIZE = 512; 

    envCubemap = createCubemap(ENV_SIZE, GL_RGB16F, true); 

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, ENV_SIZE, ENV_SIZE); 

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,

                              GL_RENDERBUFFER, captureRBO); 

    equirectShader->use(); 

    equirectShader->setInt("equirectangularMap", 0); 

    equirectShader->setMat4("projection", kCaptureProjection); 

    glActiveTexture(GL_TEXTURE0); 

    glBindTexture(GL_TEXTURE_2D, hdrTexture); 

    glViewport(0, 0, ENV_SIZE, ENV_SIZE); 

    for (int face = 0; face < 6; ++face) 

    { 

        equirectShader->setMat4("view", kCaptureViews[face]); 

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 

                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, envCubemap, 0); 

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        renderCubeForCapture(); 

    } 

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 

    irradianceShader = std::make_unique<Shader>("Shaders/cubemap_capture_vertex.glsl",

                                                "Shaders/irradiance_convolution_fragment.glsl"); 

    const int IRR_SIZE = 32; 

    irradianceMap = createCubemap(IRR_SIZE, GL_RGB16F); 

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, IRR_SIZE, IRR_SIZE); 

    irradianceShader->use(); 

    irradianceShader->setInt("environmentMap", 0); 

    irradianceShader->setMat4("projection", kCaptureProjection); 

    glActiveTexture(GL_TEXTURE0); 

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 

    glViewport(0, 0, IRR_SIZE, IRR_SIZE); 

    for (int face = 0; face < 6; ++face) 

    { 

        irradianceShader->setMat4("view", kCaptureViews[face]); 

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 

                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, irradianceMap, 0); 

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

        renderCubeForCapture(); 

    } 

    prefilterShader = std::make_unique<Shader>("Shaders/cubemap_capture_vertex.glsl",

                                               "Shaders/prefilter_fragment.glsl"); 

    const int PF_SIZE = 128; 

    const int MAX_MIP = 5; 

    prefilterMap = createCubemap(PF_SIZE, GL_RGB16F, true); 

    prefilterShader->use(); 

    prefilterShader->setInt("environmentMap", 0); 

    prefilterShader->setMat4("projection", kCaptureProjection); 

    prefilterShader->setFloat("envResolution", (float)ENV_SIZE); 

    glActiveTexture(GL_TEXTURE0); 

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 

    for (int mip = 0; mip < MAX_MIP; ++mip) 

    { 

        int mipSize = (int)(PF_SIZE * std::pow(0.5f, mip)); 

        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipSize, mipSize); 

        glViewport(0, 0, mipSize, mipSize); 

        prefilterShader->setFloat("roughness", (float)mip / (MAX_MIP - 1)); 

        for (int face = 0; face < 6; ++face) 

        { 

            prefilterShader->setMat4("view", kCaptureViews[face]); 

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 

                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, prefilterMap, mip); 

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

            renderCubeForCapture(); 

        } 

    } 

    glGenVertexArrays(1, &brdfQuadVAO);

    glGenBuffers(1, &brdfQuadVBO);

    glBindVertexArray(brdfQuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, brdfQuadVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    glBindVertexArray(0);

    brdfShader = std::make_unique<Shader>("Shaders/brdf_vertex.glsl",

                                          "Shaders/brdf_fragment.glsl"); 

    glGenTextures(1, &brdfLUTTexture); 

    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture); 

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, nullptr); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,

                           GL_TEXTURE_2D, brdfLUTTexture, 0); 

    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); 

    glViewport(0, 0, 512, 512); 

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    brdfShader->use(); 

    renderQuadForCapture(); 

    skyboxShader = std::make_unique<Shader>("Shaders/skybox_vertex.glsl",

                                            "Shaders/skybox_fragment.glsl"); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 

    hdriLoaded = true; 

    std::cout << "[HDRI] IBL precomputation complete." << std::endl; 

} 

void RenderSystem::setupDebugQuad() 

{ 

    debugShader = std::make_unique<Shader>("Shaders/shadowmap_debug_vertex.glsl",

                                           "Shaders/shadowmap_debug_fragment.glsl"); 

    glGenVertexArrays(1, &quadVAO); 

    glGenBuffers(1, &quadVBO); 

    glBindVertexArray(quadVAO); 

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO); 

    glBufferData(GL_ARRAY_BUFFER, sizeof(kQuadVerts), kQuadVerts, GL_STATIC_DRAW); 

    glEnableVertexAttribArray(0); 

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0); 

    glEnableVertexAttribArray(1); 

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float))); 

    glBindVertexArray(0); 

} 

void RenderSystem::renderShadowMapDebug() 

{ 

    glViewport(0, 0, 400, 400); 

    debugShader->use(); 

    glActiveTexture(GL_TEXTURE0); 

    glBindTexture(GL_TEXTURE_2D, shadowDepthMap); 

    debugShader->setInt("depthMap", 0); 

    glBindVertexArray(quadVAO); 

    glDisable(GL_DEPTH_TEST); 

    glDrawArrays(GL_TRIANGLES, 0, 6); 

    glEnable(GL_DEPTH_TEST); 

    glBindVertexArray(0); 

} 

void RenderSystem::ShadowPass(float currentFrame,

                              glm::mat4 lightSpaceMatrix,

                              const glm::vec3& cameraPos) 

{ 

    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f); 

    lightPos        = glm::vec3(0.0f, 70.0f, 0.0f); 

    lightView       = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f)); 

    lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 200.0f); 

    lightDir        = glm::normalize(lightTarget - lightPos); 

    lightSpaceMatrix = lightProjection * lightView; 

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO); 

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); 

    glClear(GL_DEPTH_BUFFER_BIT); 

    if (shadowShader) 

    { 

        shadowShader->use(); 

        shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix); 

        const float SHADOW_CULL = 30.0f; 

        for (auto& modelData : modelLoader->models) 

        { 

            glm::vec3 wbc = glm::vec3(

                modelData.transform * glm::vec4(modelData.model->boundsCenter, 1.0f)); 

            if (glm::distance(cameraPos, wbc) >

                modelData.model->boundsRadius + SHADOW_CULL) continue; 

            shadowShader->setMat4("model", modelData.transform); 

            modelData.model->draw(*shadowShader, modelData.transform); 

        } 

    } 

    glEnable(GL_CULL_FACE); 

    glCullFace(GL_BACK); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

} 

void RenderSystem::RenderPass(const Camera &camera,

                              const glm::mat4 &view,

                              const glm::mat4 &projection,

                              glm::mat4 lightSpaceMatrix,

                              float currentFrame) 

{ 

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 

    glClearColor(0.52f, 0.84f, 0.92f, 1.0f); 

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE); 

    glCullFace(GL_BACK); 

    if (modelShader) 

    { 

        modelShader->use(); 

        modelShader->setVec3("cameraPos",  camera.Position); 

        modelShader->setMat4("view",       view); 

        modelShader->setMat4("projection", projection); 

        modelShader->setMat4("lightSpaceMatrix", lightSpaceMatrix); 

        modelShader->setVec3("lightDir",   lightDir); 

        modelShader->setVec3("fogColor",   glm::vec3(0.0f, 0.0f, 0.0f)); 

        modelShader->setFloat("fogStart",  200.0f); 

        modelShader->setFloat("fogEnd",    500.0f); 

        

        

        

        

        

        

        

        

        

        

        modelShader->setInt("shadowMap", 1); 

        glActiveTexture(GL_TEXTURE1); 

        glBindTexture(GL_TEXTURE_2D, shadowDepthMap); 

        if (hdriLoaded) 

        { 

            modelShader->setBool("hasIBL",        true); 

            modelShader->setInt("irradianceMap",  2); 

            modelShader->setInt("prefilterMap",   3); 

            modelShader->setInt("brdfLUT",        4); 

            glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); 

            glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); 

            glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, brdfLUTTexture); 

        } 

        else 

        { 

            modelShader->setBool("hasIBL", false); 

        } 

        

        modelShader->setFloat("materialMetallic",  0.0f); 

        modelShader->setFloat("materialRoughness", 0.5f); 

        modelShader->setFloat("materialAO",        1.0f); 

        

        

        modelShader->setVec3("materialEmissive",   glm::vec3(0.0f));

        modelShader->setFloat("emissiveStrength",  1.0f);   

        

        if (m_lightManager)

        {

            int numActive = 0;

            for (int i = 0; i < m_lightManager->count() && numActive < MAX_LIGHTS; ++i)

            {

                const Light& l = m_lightManager->get(i);

                if (!l.enabled) continue;

                std::string base = "u_lights[" + std::to_string(numActive) + "].";

                modelShader->setInt  (base + "type",        (int)l.type);

                

                modelShader->setVec3 (base + "color",       l.color * l.intensity);

                modelShader->setVec3 (base + "position",    l.position);

                modelShader->setVec3 (base + "direction",   glm::normalize(l.direction));

                modelShader->setFloat(base + "constant",    l.constant);

                modelShader->setFloat(base + "linear",      l.linear);

                modelShader->setFloat(base + "quadratic",   l.quadratic);

                

                modelShader->setFloat(base + "innerCutoff", std::cos(glm::radians(l.innerCutoff)));

                modelShader->setFloat(base + "outerCutoff", std::cos(glm::radians(l.outerCutoff)));

                ++numActive;

            }

            modelShader->setInt("u_numLights", numActive);

        }

        else

        {

            modelShader->setInt("u_numLights", 0);

        }

        const float CULL_DISTANCE = 90.0f; 

        for (size_t i = 0; i < modelLoader->models.size(); i++) 

        { 

            auto& modelData = modelLoader->models[i]; 

            glm::vec3 wbc = glm::vec3(

                modelData.transform * glm::vec4(modelData.model->boundsCenter, 1.0f)); 

            if (glm::distance(camera.Position, wbc) >

                modelData.model->boundsRadius + CULL_DISTANCE) continue; 

            modelData.model->draw(*modelShader, modelData.transform, 

                                  modelLoader->getBoneMatrices(i), true); 

        } 

    } 

    if (bulletDebugDrawEnabled && physicsWorld && bulletDebugDrawer && debugLineShader) 

    { 

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

        glLineWidth(2.0f); 

        bulletDebugDrawer->setShaderAndMatrices(debugLineShader.get(), view, projection); 

        physicsWorld->setDebugDrawer(bulletDebugDrawer.get()); 

        physicsWorld->debugDrawWorld(); 

        glLineWidth(1.0f); 

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 

        glEnable(GL_CULL_FACE); 

    } 

    renderSkybox(view, projection); 

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

} 

void RenderSystem::renderSkybox(const glm::mat4 &view, const glm::mat4 &projection) 

{ 

    if (!hdriLoaded || !skyboxShader) return; 

    glDepthMask(GL_FALSE); 

    glDisable(GL_CULL_FACE); 

    skyboxShader->use(); 

    skyboxShader->setMat4("view",       view); 

    skyboxShader->setMat4("projection", projection); 

    skyboxShader->setInt("environmentMap", 0); 

    skyboxShader->setFloat("exposure", hdriExposure); 

    glActiveTexture(GL_TEXTURE0); 

    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 

    renderCubeForCapture(); 

    glEnable(GL_CULL_FACE); 

    glDepthMask(GL_TRUE); 

} 

void RenderSystem::render(const Camera &camera, float currentFrame,

                          const glm::mat4 &view, const glm::mat4 &projection) 

{ 

    if (modelShader)  modelShader->hotReload(); 

    if (shadowShader) shadowShader->hotReload(); 

    ShadowPass(currentFrame, lightProjection * lightView, camera.Position); 

    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 

    RenderPass(camera, view, projection, lightProjection * lightView, currentFrame); 

    BloomPass();

    CompositePass();

}

void RenderSystem::resizeBloomBuffers(int width, int height)

{

    screenWidth = width;

    screenHeight = height;

    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,

                 width, height,

                 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, brightColorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,

                 width, height,

                 0, GL_RGBA, GL_FLOAT, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, hdrRBO);

    glRenderbufferStorage(GL_RENDERBUFFER,

                          GL_DEPTH_COMPONENT24,

                          width, height);

    for(int i = 0; i < 2; i++)

    {

        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,

                     width, height,

                     0, GL_RGBA, GL_FLOAT, nullptr);

    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

}