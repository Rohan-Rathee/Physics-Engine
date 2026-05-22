#include "render_system.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

const float cubeVertices[] = {

    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,


    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,


    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,


     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,


    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,


    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
};
std::vector<glm::vec3> cubePositions = {
    glm::vec3(-1.0f,  0.0f, -1.0f),
    glm::vec3( 2.0f,  0.0f, -1.0f),
    glm::vec3(-1.0f,  0.0f,  2.0f),
    glm::vec3( 2.0f,  0.0f,  2.0f)
};

RenderSystem::RenderSystem(const std::string& vertexPath, const std::string& fragmentPath, unsigned int width, unsigned int height)
    : shader(nullptr), modelShader(nullptr), modelLoader(std::make_unique<ModelLoader>()), VAO(0), VBO(0), 
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
    
    modelShader = std::make_unique<Shader>(modelVertexPath.c_str(), modelFragmentPath.c_str());
    loadModel("models/fps_shooter_game_arena_map_v3.glb", glm::vec3(0.0f), glm::vec3(1.0f));

    setModelTransform(0, glm::vec3(+20.0f, -10.0f, +20.0f), glm::vec3(1.0f), 90.0f, glm::vec3(-1.0f, 0.0f, 0.0f));

    modelShader->use();
}
void RenderSystem::setupCube() {

    shader = std::make_unique<Shader>(vertexPath.c_str(), fragmentPath.c_str());

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
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "Failed to load texture1" << std::endl;
        return;
    }

    stbi_image_free(data);
    
    shader->use();
    shader->setInt("texture1", 0);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void RenderSystem::setupShadowFramebuffer(){
    shadowShader = std::make_unique<Shader>("Shaders/shadow_vertex.glsl", "Shaders/shadow_fragment.glsl");
    SHADOW_WIDTH = 10240;
    SHADOW_HEIGHT = 10240;

    glGenFramebuffers(1, &shadowFBO);


    glGenTextures(1, &shadowDepthMap);
    glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);



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

    return true;
}

void RenderSystem::ShadowPass(float currentFrame, glm::mat4 lightSpaceMatrix) {
    lightPos = glm::vec3(10.0f * cos(currentFrame), 30.0f, 10.0f * sin(currentFrame));
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 30.0f, 0.1f, 50.0f);
    lightDir = glm::normalize(lightTarget - lightPos);
    lightSpaceMatrix = lightProjection * lightView;
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

    glClear(GL_DEPTH_BUFFER_BIT);


    if (shadowShader) {
        shadowShader->use();
        shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        if (!modelLoader->models.empty()) {
            for (auto& modelData : modelLoader->models) {
                shadowShader->setMat4("model", modelData.transform);
                modelData.model->draw(*shadowShader);
            }
        }
        
        glBindVertexArray(VAO);
        for (const auto& pos : cubePositions) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(0.5f));
            shadowShader->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    
    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
}
void RenderSystem::RenderPass(const glm::mat4& view, const glm::mat4& projection, glm::mat4 lightSpaceMatrix, float currentFrame) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 lightSpaceMatrix_computed = lightProjection * lightView;
    
    if (modelShader && !modelLoader->models.empty()) {
        modelShader->use();
        modelShader->setMat4("view", view);
        modelShader->setMat4("projection", projection);
        modelShader->setMat4("lightSpaceMatrix", lightSpaceMatrix_computed);
        modelShader->setVec3("lightDir", lightDir);
        modelShader->setInt("texture_diffuse1", 0);
        modelShader->setInt("shadowMap", 1);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
        
        for (auto& modelData : modelLoader->models) {
            modelShader->setMat4("model", modelData.transform);
            modelData.model->draw(*modelShader);
        }
    }

    if (shader) {
        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        shader->setMat4("lightSpaceMatrix", lightSpaceMatrix_computed);
        shader->setVec3("lightDir", lightDir);
        shader->setInt("texture1", 0);
        shader->setInt("shadowMap", 1);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);

        glBindVertexArray(VAO);
        for (const auto& pos : cubePositions) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(0.5f));
            shader->setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
}


void RenderSystem::render(float currentFrame, const glm::mat4& view, const glm::mat4& projection) {
    if (shader) shader->hotReload();
    if (modelShader) modelShader->hotReload();
    if (shadowShader) shadowShader->hotReload();
    
    ShadowPass(currentFrame, lightProjection * lightView);
    RenderPass(view, projection, lightProjection * lightView, currentFrame);
}

