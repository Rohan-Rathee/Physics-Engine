#include "render_system.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); // Example light direction for shadow calculation

const float cubeVertices[] = {    // Back face (z = -0.5) — FIXED (reversed)

    // Back face (z = -0.5)
-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
-0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
 0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
 0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
-0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,

// Front face (z = +0.5)
-0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
 0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
 0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
 0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
-0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
-0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

// Left face (x = -0.5)
-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
-0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
-0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
-0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
-0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

// Right face (x = +0.5)
 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
 0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
 0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
 0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
 0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

// Bottom face (y = -0.5)
-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
 0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
 0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
-0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
-0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

// Top face (y = +0.5)
-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
-0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
 0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
 0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
-0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
};
float quadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};


RenderSystem::RenderSystem(const std::string& vertexPath, const std::string& fragmentPath, unsigned int width, unsigned int height)
    : modelShader(nullptr), modelLoader(std::make_unique<ModelLoader>()), VAO(0), VBO(0), 
      texture1(0), screenWidth(width), screenHeight(height),
      vertexPath(vertexPath), fragmentPath(fragmentPath),
      modelVertexPath("Shaders/model_vertex.glsl"), modelFragmentPath("Shaders/model_fragment.glsl"),
      shadowFBO(0), shadowDepthMap(0), shadowShader(nullptr) {}
RenderSystem::~RenderSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowDepthMap);
}
void RenderSystem::setupModels() {
    loadModel("models/Untitled.glb", glm::vec3(0.0f), glm::vec3(1.0f));
    setModelTransform(0, glm::vec3(0.0f, -2.0f , 0.0f), glm::vec3(1.0f), 00.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    
    //id, position (left, up, forward), scale, rotationAngle, rotationAxis
}


void RenderSystem::setupCube() {

    modelShader = std::make_unique<Shader>(modelVertexPath.c_str(), modelFragmentPath.c_str());

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("Textures/prototype_grid_grey.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        //PARAMETERS PASSED: target, level, internalFormat, width, height, border, format, type, data
        //WHAT COULD CAUSE  BLURRING OF SHARP TEXTURES: Using GL_LINEAR filtering without mipmaps can cause blurring. Since we are using GL_NEAREST, this should not be the issue. However, if mipmaps were generated and GL_LINEAR_MIPMAP_LINEAR was used, it could cause blurring if the texture is minified.
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture1" << std::endl;
        return;
    }

    stbi_image_free(data);
    
    modelShader->use();
    modelShader->setInt("texture_diffuse1", 0);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    //PARAMETERS PASSED: target, size, data, usage
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    //PARAMETERS PASSED: index, size, type, normalized, stride, pointer
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

}
void RenderSystem::setupShadowFramebuffer(){
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer is not complete!" << std::endl;
    }
    
    lightPos = glm::vec3(10.0f, 10.0f, 10.0f);
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));   
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 50.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
bool RenderSystem::initialize(){

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    setupCube();
    setupModels();
    setupShadowFramebuffer();
    setupDebugQuad();

    return true;
}

void RenderSystem::setupDebugQuad()
{
    debugShader = std::make_unique<Shader>("Shaders/shadowmap_debug_vertex.glsl","Shaders/shadowmap_debug_fragment.glsl");

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quadVertices),&quadVertices,GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)0 );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2,GL_FLOAT,GL_FALSE,4 * sizeof(float),(void*)(2 * sizeof(float)));

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
void RenderSystem::ShadowPass(float currentFrame, glm::mat4 lightSpaceMatrix) {
    //woudlnt sunrise/sunset loop be better
    //lightPos = glm::vec3(10.0f * cos(currentFrame), 10.0f * sin(currentFrame), 10.0f);
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    //lets make lightpos orbit around light target, so we can see the shadows changing dynamically. We can use a simple circular orbit for this
    //whats the difference between lighttarget adn light
    lightPos = glm::vec3(10.0f * sin(currentFrame), 30.0f, 10.0f * cos(currentFrame));
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 100.0f);
    lightDir = glm::normalize(lightTarget - lightPos);
    lightSpaceMatrix = lightProjection * lightView;
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    glClear(GL_DEPTH_BUFFER_BIT);
    //glDisable(GL_CULL_FACE);

    if (shadowShader) {
        shadowShader->use();
        shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        if (!modelLoader->models.empty()) {
            for (auto& modelData : modelLoader->models) {
                shadowShader->setMat4("model", modelData.transform);
                modelData.model->draw(*shadowShader, modelData.transform);
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
void RenderSystem::RenderPass(const Camera& camera, const glm::mat4& view, const glm::mat4& projection, glm::mat4 lightSpaceMatrix, float currentFrame) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
    glClearColor(0.52f, 0.84f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 lightSpaceMatrix_computed = lightProjection * lightView;
    
    if (modelShader) {
        modelShader->use();
        modelShader->setVec3("cameraPos", camera.Position); 
        modelShader->setMat4("view", view);
        modelShader->setMat4("projection", projection);
        modelShader->setMat4("lightSpaceMatrix", lightSpaceMatrix_computed);
        modelShader->setVec3("lightDir", lightDir);
        modelShader->setInt("texture_diffuse1", 0);
        modelShader->setInt("shadowMap", 1);
        
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);

        if (!modelLoader->models.empty()) {
            for (auto& modelData : modelLoader->models) {
                modelData.model->draw(*modelShader, modelData.transform);
        }
        }

    }
    //renderShadowMapDebug();
}


void RenderSystem::render(const Camera& camera, float currentFrame, const glm::mat4& view, const glm::mat4& projection) {
    

    //id, position, scale, rotationAngle, rotationAxis
    
    if (modelShader) modelShader->hotReload();
    if (shadowShader) shadowShader->hotReload();
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
    ShadowPass(currentFrame, lightProjection * lightView);
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
    RenderPass(camera, view, projection, lightProjection * lightView, currentFrame);
    setupDebugQuad();
    renderShadowMapDebug();
}
