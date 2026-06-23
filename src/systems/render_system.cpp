#include "render_system.h" 
#include <iostream> 
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h" 
#include <filesystem> 
namespace fs = std::filesystem; 
glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); 
static const float kSkyboxVertices[] = { 
      
      
    -1.0f,  1.0f, -1.0f, 
    -1.0f, -1.0f, -1.0f, 
     1.0f, -1.0f, -1.0f, 
     1.0f, -1.0f, -1.0f, 
     1.0f,  1.0f, -1.0f, 
    -1.0f,  1.0f, -1.0f, 
      
    -1.0f, -1.0f,  1.0f, 
    -1.0f, -1.0f, -1.0f, 
    -1.0f,  1.0f, -1.0f, 
    -1.0f,  1.0f, -1.0f, 
    -1.0f,  1.0f,  1.0f, 
    -1.0f, -1.0f,  1.0f, 
      
     1.0f, -1.0f, -1.0f, 
     1.0f, -1.0f,  1.0f, 
     1.0f,  1.0f,  1.0f, 
     1.0f,  1.0f,  1.0f, 
     1.0f,  1.0f, -1.0f, 
     1.0f, -1.0f, -1.0f, 
      
    -1.0f, -1.0f,  1.0f, 
    -1.0f,  1.0f,  1.0f, 
     1.0f,  1.0f,  1.0f, 
     1.0f,  1.0f,  1.0f, 
     1.0f, -1.0f,  1.0f, 
    -1.0f, -1.0f,  1.0f, 
      
    -1.0f,  1.0f, -1.0f, 
     1.0f,  1.0f, -1.0f, 
     1.0f,  1.0f,  1.0f, 
     1.0f,  1.0f,  1.0f, 
    -1.0f,  1.0f,  1.0f, 
    -1.0f,  1.0f, -1.0f, 
      
    -1.0f, -1.0f, -1.0f, 
    -1.0f, -1.0f,  1.0f, 
     1.0f, -1.0f, -1.0f, 
     1.0f, -1.0f, -1.0f, 
    -1.0f, -1.0f,  1.0f, 
     1.0f, -1.0f,  1.0f 
}; 
void RenderSystem::loadEnvironmentFolder(){ 
    std::string envFolder = "models/ENV/"; 
    for (const auto& entry : fs::directory_iterator(envFolder)) 
    { 
        if (!entry.is_regular_file()) 
            continue; 
        std::string path = entry.path().string(); 
          
        if (entry.path().extension() == ".glb" || 
            entry.path().extension() == ".gltf" || 
            entry.path().extension() == ".fbx" || 
            entry.path().extension() == ".obj") 
        { 
            loadModel(path, glm::vec3(0.0f), glm::vec3(1.0f)); 
            int index = modelLoader->models.size() - 1; 
            setModelTransform( 
                index, 
                glm::vec3(0.0f), 
                glm::vec3(1.0f), 
                0.0f, 
                glm::vec3(0.0f, 1.0f, 0.0f) 
            ); 
            btCollisionShape* shape = 
            modelLoader->models[index].model->buildTriangleMeshCollider(); 
            modelTransform->initializePhysicsBody( 
                index, 
                0.0f, 
                shape, 
                0.2f 
            ); 
        } 
    } 
} 
static const glm::mat4 kCaptureViews[6] = { 
    glm::lookAt(glm::vec3(0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)), 
    glm::lookAt(glm::vec3(0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)), 
    glm::lookAt(glm::vec3(0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)), 
    glm::lookAt(glm::vec3(0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)), 
    glm::lookAt(glm::vec3(0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)), 
    glm::lookAt(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)), 
}; 
static const glm::mat4 kCaptureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f); 
float quadVertices[] = { 
      
    -1.0f, 1.0f, 0.0f, 1.0f, 
    -1.0f, -1.0f, 0.0f, 0.0f, 
    1.0f, -1.0f, 1.0f, 0.0f, 
    -1.0f, 1.0f, 0.0f, 1.0f, 
    1.0f, -1.0f, 1.0f, 0.0f, 
    1.0f, 1.0f, 1.0f, 1.0f}; 
RenderSystem::RenderSystem(const std::string &vertexPath, const std::string &fragmentPath, unsigned int width, unsigned int height) 
    : modelShader(nullptr), modelLoader(std::make_unique<ModelLoader>()), modelTransform(nullptr), VAO(0), VBO(0), 
      texture1(0), screenWidth(width), screenHeight(height), 
      vertexPath(vertexPath), fragmentPath(fragmentPath), 
      modelVertexPath("Shaders/model_vertex.glsl"), modelFragmentPath("Shaders/model_fragment.glsl"), 
      shadowFBO(0), shadowDepthMap(0), shadowShader(nullptr), 
      bulletDebugDrawer(std::make_unique<BulletDebugDrawer>()), physicsWorld(nullptr) 
{ 
      
} 
RenderSystem::~RenderSystem() 
{ 
    glDeleteVertexArrays(1, &VAO); 
    glDeleteBuffers(1, &VBO); 
    glDeleteFramebuffers(1, &shadowFBO); 
    glDeleteTextures(1, &shadowDepthMap); 
    if (skyboxVAO) 
    { 
        glDeleteVertexArrays(1, &skyboxVAO); 
    } 
    if (skyboxVBO) 
    { 
        glDeleteBuffers(1, &skyboxVBO); 
    } 
    if (captureFBO) 
    { 
        glDeleteFramebuffers(1, &captureFBO); 
    } 
    if (captureRBO) 
    { 
        glDeleteRenderbuffers(1, &captureRBO); 
    } 
    if (hdrTexture) 
    { 
        glDeleteTextures(1, &hdrTexture); 
    } 
    if (envCubemap) 
    { 
        glDeleteTextures(1, &envCubemap); 
    } 
    if (irradianceMap) 
    { 
        glDeleteTextures(1, &irradianceMap); 
    } 
    if (prefilterMap) 
    { 
        glDeleteTextures(1, &prefilterMap); 
    } 
    if (brdfLUTTexture) 
    { 
        glDeleteTextures(1, &brdfLUTTexture); 
    } 
} 
unsigned int RenderSystem::createCubemap(int size, GLenum internalFormat, bool mipmap) 
{ 
    unsigned int id; 
    glGenTextures(1, &id); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, id); 
    GLenum fmt = (internalFormat == GL_RGB16F || internalFormat == GL_RG16F) 
                     ? GL_RGB 
                     : GL_RGB; 
    GLenum type = (internalFormat == GL_RGB16F || internalFormat == GL_RG16F) 
                      ? GL_FLOAT 
                      : GL_UNSIGNED_BYTE; 
      
    if (internalFormat == GL_RG16F) 
        fmt = GL_RG; 
    for (int face = 0; face < 6; ++face) 
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 
                     internalFormat, size, size, 0, fmt, type, nullptr); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, 
                    mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    if (mipmap) 
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 
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
    glBindVertexArray(quadVAO); 
    glDrawArrays(GL_TRIANGLES, 0, 6); 
    glBindVertexArray(0); 
} 
void RenderSystem::setupModels() 
{ 
      
    btCollisionShape *basketballShape = nullptr; 
    for (int i = 0; i < 1; ++i) 
    { 
        for (int j = 0; j < 1; ++j) 
        { 
            loadModel("models\\untitled1.glb", glm::vec3(0.0f), glm::vec3(1.0f)); 
            int currentIndex = modelLoader->models.size() - 1; 
              
              
            modelLoader->setModelAnimation(currentIndex, 0); 
            if (basketballShape == nullptr && modelTransform) 
            { 
                Model *basketballModel = modelLoader->models[currentIndex].model.get(); 
                basketballShape = basketballModel->buildCapsuleColliderFromMesh(); 
                glm::vec3 scale = 
                    modelLoader->getModelScale(currentIndex);   
                basketballShape->setLocalScaling( 
                    btVector3( 
                        scale.x, 
                        scale.y, 
                        scale.z)); 
            } 
            glm::vec3 position = glm::vec3(0.0f, 10.0f, 0.0f); 
            setModelTransform(currentIndex, position, glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)); 
            if (modelTransform && basketballShape) 
            { 
                modelTransform->initializePhysicsBody(currentIndex, 80.0f, basketballShape, 0.1f); 
                  
            } 
        } 
    } 
    loadEnvironmentFolder(); 
}   
void RenderSystem::setupShadowFramebuffer() 
{ 
    shadowShader = std::make_unique<Shader>("Shaders/shadow_vertex.glsl", "Shaders/shadow_fragment.glsl"); 
    glGenFramebuffers(1, &shadowFBO); 
    glGenTextures(1, &shadowDepthMap); 
    glBindTexture(GL_TEXTURE_2D, shadowDepthMap); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO); 
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f}; 
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthMap, 0); 
    glDrawBuffer(GL_NONE); 
    glReadBuffer(GL_NONE); 
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
    { 
        std::cerr << "Shadow framebuffer is not complete!" << std::endl; 
    } 
    lightPos = glm::vec3(10.0f, 10.0f, 10.0f); 
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f); 
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f)); 
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 50.0f); 
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
        modelShader = std::make_unique<Shader>( 
        modelVertexPath.c_str(), 
        modelFragmentPath.c_str()); 
    setupShadowFramebuffer(); 
    setupDebugQuad(); 
    bulletDebugDrawer->initBuffers(); 
    debugLineShader = std::make_unique<Shader>("Shaders/debug_line_vertex.glsl", "Shaders/debug_line_fragment.glsl"); 
    setupHDRI("models\\HDR_galactic_plane_3.hdr"); 
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(kSkyboxVertices), kSkyboxVertices, 
                 GL_STATIC_DRAW); 
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0); 
    glBindVertexArray(0); 
      
    stbi_set_flip_vertically_on_load(true); 
    int w, h, channels; 
    float *data = stbi_loadf(hdrPath.c_str(), &w, &h, &channels, 0); 
    if (!data) 
    { 
        std::cerr << "[HDRI] Failed to load: " << hdrPath << std::endl; 
        return; 
    } 
    glGenTextures(1, &hdrTexture); 
    glBindTexture(GL_TEXTURE_2D, hdrTexture); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, w, h, 0, 
                 GL_RGB, GL_FLOAT, data); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    stbi_image_free(data); 
    std::cout << "[HDRI] Loaded " << hdrPath 
              << " (" << w << "x" << h << ")" << std::endl; 
      
    glGenFramebuffers(1, &captureFBO); 
    glGenRenderbuffers(1, &captureRBO); 
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO); 
      
    equirectShader = std::make_unique<Shader>( 
        "Shaders/cubemap_capture_vertex.glsl", 
        "Shaders/equirect_to_cubemap_fragment.glsl"); 
    const int ENV_SIZE = 512; 
    envCubemap = createCubemap(ENV_SIZE, GL_RGB16F, /*mipmap=*/true); 
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 
                          ENV_SIZE, ENV_SIZE); 
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
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                               envCubemap, 0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        renderCubeForCapture(); 
    } 
      
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); 
      
    irradianceShader = std::make_unique<Shader>( 
        "Shaders/cubemap_capture_vertex.glsl", 
        "Shaders/irradiance_convolution_fragment.glsl"); 
    const int IRR_SIZE = 32; 
    irradianceMap = createCubemap(IRR_SIZE, GL_RGB16F); 
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 
                          IRR_SIZE, IRR_SIZE); 
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
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                               irradianceMap, 0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
        renderCubeForCapture(); 
    } 
      
    prefilterShader = std::make_unique<Shader>( 
        "Shaders/cubemap_capture_vertex.glsl", 
        "Shaders/prefilter_fragment.glsl"); 
    const int PF_SIZE = 128; 
    const int MAX_MIP = 5; 
    prefilterMap = createCubemap(PF_SIZE, GL_RGB16F, /*mipmap=*/true); 
    prefilterShader->use(); 
    prefilterShader->setInt("environmentMap", 0); 
    prefilterShader->setMat4("projection", kCaptureProjection); 
    prefilterShader->setFloat("envResolution", 
                              static_cast<float>(ENV_SIZE)); 
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 
    for (int mip = 0; mip < MAX_MIP; ++mip) 
    { 
        int mipSize = static_cast<int>(PF_SIZE * std::pow(0.5f, mip)); 
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO); 
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 
                              mipSize, mipSize); 
        glViewport(0, 0, mipSize, mipSize); 
        float roughness = static_cast<float>(mip) / (MAX_MIP - 1); 
        prefilterShader->setFloat("roughness", roughness); 
        for (int face = 0; face < 6; ++face) 
        { 
            prefilterShader->setMat4("view", kCaptureViews[face]); 
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 
                                   prefilterMap, mip); 
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
            renderCubeForCapture(); 
        } 
    } 
      
    brdfShader = std::make_unique<Shader>( 
        "Shaders/brdf_vertex.glsl", 
        "Shaders/brdf_fragment.glsl"); 
    glGenTextures(1, &brdfLUTTexture); 
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture); 
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, 
                 GL_RG, GL_FLOAT, nullptr); 
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
      
    skyboxShader = std::make_unique<Shader>( 
        "Shaders/skybox_vertex.glsl", 
        "Shaders/skybox_fragment.glsl"); 
      
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 
    hdriLoaded = true; 
    std::cout << "[HDRI] IBL precomputation complete." << std::endl; 
} 
void RenderSystem::setupDebugQuad() 
{ 
    debugShader = std::make_unique<Shader>("Shaders/shadowmap_debug_vertex.glsl", "Shaders/shadowmap_debug_fragment.glsl"); 
    glGenVertexArrays(1, &quadVAO); 
    glGenBuffers(1, &quadVBO); 
    glBindVertexArray(quadVAO); 
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW); 
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0); 
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float))); 
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
void RenderSystem::ShadowPass( 
    float currentFrame, 
    glm::mat4 lightSpaceMatrix, 
    const glm::vec3& cameraPos) 
{ 
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f); 
    lightPos = glm::vec3(0.0f, 70.0f, 0.0f); 
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f)); 
    lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 100.0f); 
    lightDir = glm::normalize(lightTarget - lightPos); 
    lightSpaceMatrix = lightProjection * lightView; 
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO); 
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT); 
    glClear(GL_DEPTH_BUFFER_BIT); 
      
    if (shadowShader) 
    { 
        shadowShader->use(); 
        shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix); 
        if (!modelLoader->models.empty()) 
        { 
            const float SHADOW_CULL_DISTANCE = 30.0f; 
            for (auto& modelData : modelLoader->models) 
            { 
                  
                  
                  
                glm::vec3 worldBoundsCenter = glm::vec3( 
                    modelData.transform * glm::vec4(modelData.model->boundsCenter, 1.0f)); 
                float distance = 
                    glm::distance(cameraPos, worldBoundsCenter); 
                if (distance > modelData.model->boundsRadius + SHADOW_CULL_DISTANCE) 
                    continue; 
                shadowShader->setMat4("model", modelData.transform); 
                modelData.model->draw( 
                    *shadowShader, 
                    modelData.transform 
                ); 
            } 
        } 
        glBindVertexArray(VAO); 
    } 
    glBindVertexArray(0); 
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glClear(GL_DEPTH_BUFFER_BIT); 
} 
void RenderSystem::RenderPass(const Camera &camera, const glm::mat4 &view, const glm::mat4 &projection, glm::mat4 lightSpaceMatrix, float currentFrame) 
{ 
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 
    glClearColor(0.52f, 0.84f, 0.92f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK); 
    glm::mat4 lightSpaceMatrix_computed = lightProjection * lightView; 
    if (modelShader) 
    { 
        modelShader->use(); 
        if (hdriLoaded) 
                 
        { 
            modelShader->setBool("hasIBL", true); 
            modelShader->setFloat("ao", 1.0f); 
            modelShader->setFloat("metallic", 0.0f); 
            modelShader->setFloat("roughness", 0.5f); 
            modelShader->setInt("irradianceMap", 2); 
            modelShader->setInt("prefilterMap", 3); 
            modelShader->setInt("brdfLUT", 4); 
            glActiveTexture(GL_TEXTURE2); 
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); 
            glActiveTexture(GL_TEXTURE3); 
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); 
            glActiveTexture(GL_TEXTURE4); 
            glBindTexture(GL_TEXTURE_2D, brdfLUTTexture); 
        } 
        else 
        { 
            modelShader->setBool("hasIBL", false); 
        } 
        modelShader->setVec3("cameraPos", camera.Position); 
        modelShader->setMat4("view", view); 
        modelShader->setMat4("projection", projection); 
        modelShader->setMat4("lightSpaceMatrix", lightSpaceMatrix_computed); 
        modelShader->setVec3("lightDir", lightDir); 
        modelShader->setInt("texture_diffuse1", 0); 
        modelShader->setInt("shadowMap", 1); 
        modelShader->setVec3( 
            "fogColor", 
            glm::vec3( 
0.52f, 0.84f, 0.92f
            )   
        ); 
        modelShader->setFloat("fogStart", 20.0f);            
        modelShader->setFloat("fogEnd", 50.0f); 
        if (hdriLoaded) 
        { 
            modelShader->setInt("irradianceMap", 2);     
            modelShader->setInt("prefilterMap", 3); 
            modelShader->setInt("brdfLUT", 4); 
            glActiveTexture(GL_TEXTURE2); 
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); 
            glActiveTexture(GL_TEXTURE3); 
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); 
            glActiveTexture(GL_TEXTURE4); 
            glBindTexture(GL_TEXTURE_2D, brdfLUTTexture); 
        } 
        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);   
        const float CULL_DISTANCE = 90.0f; 
        for (size_t i = 0; i < modelLoader->models.size(); i++) 
        { 
            auto& modelData = modelLoader->models[i]; 
              
              
              
            glm::vec3 worldBoundsCenter = glm::vec3( 
                modelData.transform * glm::vec4(modelData.model->boundsCenter, 1.0f)); 
            float distance = 
                glm::distance(camera.Position, worldBoundsCenter); 
            if (distance > modelData.model->boundsRadius + CULL_DISTANCE) 
                continue; 
            modelData.model->draw( 
                *modelShader, 
                modelData.transform, 
                modelLoader->getBoneMatrices(i), 
                true 
            ); 
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
} 
void RenderSystem::renderSkybox(const glm::mat4 &view, const glm::mat4 &projection) 
{ 
    if (!hdriLoaded || !skyboxShader) 
        return; 
      
    glDepthMask(GL_FALSE);     
    glDisable(GL_CULL_FACE);   
    skyboxShader->use(); 
    skyboxShader->setMat4("view", view); 
    skyboxShader->setMat4("projection", projection); 
    skyboxShader->setInt("environmentMap", 0); 
    skyboxShader->setFloat("exposure", hdriExposure); 
    glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); 
    renderCubeForCapture(); 
    glEnable(GL_CULL_FACE); 
    glDepthMask(GL_TRUE); 
} 
void RenderSystem::render(const Camera &camera, float currentFrame, const glm::mat4 &view, const glm::mat4 &projection) 
{ 
      
if (modelShader) 
{ 
    modelShader->hotReload(); 
} 
    if (shadowShader) 
        shadowShader->hotReload(); 
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 
    ShadowPass( 
    currentFrame, 
    lightProjection * lightView, 
    camera.Position 
); 
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight); 
    RenderPass(camera, view, projection, lightProjection * lightView, currentFrame); 
      
      
}