#include "render_system.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


const float cubeVertices[] = {    // Back face (z = -0.5) — FIXED (reversed)
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    // Front face (z = +0.5) — OK
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    // Left face (x = -0.5) — OK
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    // Right face (x = +0.5) — FIXED (reversed)
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    // Bottom face (y = -0.5) — OK
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    // Top face (y = +0.5) — FIXED (reversed)
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
};

const glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

RenderSystem::RenderSystem(const std::string& vertexPath, const std::string& fragmentPath, unsigned int width, unsigned int height)
    : shader(nullptr), modelShader(nullptr), VAO(0), VBO(0), 
      texture1(0), texture2(0), screenWidth(width), screenHeight(height),
      vertexPath(vertexPath), fragmentPath(fragmentPath),
      modelVertexPath("Shaders/model_vertex.glsl"), modelFragmentPath("Shaders/model_fragment.glsl"),
      shadowFBO(0), shadowDepthMap(0), shadowShader(nullptr) {}

RenderSystem::~RenderSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowDepthMap);
}

void RenderSystem::extractFrustumPlanes(const glm::mat4& vp) {
    // Each plane extracted from VP matrix rows
    frustumPlanes[0] = glm::vec4(vp[0][3]+vp[0][0], vp[1][3]+vp[1][0], vp[2][3]+vp[2][0], vp[3][3]+vp[3][0]); // Left
    frustumPlanes[1] = glm::vec4(vp[0][3]-vp[0][0], vp[1][3]-vp[1][0], vp[2][3]-vp[2][0], vp[3][3]-vp[3][0]); // Right
    frustumPlanes[2] = glm::vec4(vp[0][3]+vp[0][1], vp[1][3]+vp[1][1], vp[2][3]+vp[2][1], vp[3][3]+vp[3][1]); // Bottom
    frustumPlanes[3] = glm::vec4(vp[0][3]-vp[0][1], vp[1][3]-vp[1][1], vp[2][3]-vp[2][1], vp[3][3]-vp[3][1]); // Top
    frustumPlanes[4] = glm::vec4(vp[0][3]+vp[0][2], vp[1][3]+vp[1][2], vp[2][3]+vp[2][2], vp[3][3]+vp[3][2]); // Near
    frustumPlanes[5] = glm::vec4(vp[0][3]-vp[0][2], vp[1][3]-vp[1][2], vp[2][3]-vp[2][2], vp[3][3]-vp[3][2]); // Far
}

bool RenderSystem::isInFrustum(const glm::vec3& pos) {
    for (int i = 0; i < 6; i++) {
        glm::vec4& p = frustumPlanes[i];
        if (p.x*pos.x + p.y*pos.y + p.z*pos.z + p.w <= -1.0f) // -1 adds small margin
            return false;
    }
    return true;
}

void RenderSystem::loadModel(const std::string& modelPath, const glm::vec3& position, const glm::vec3& scale) {
    try {
        auto newModel = std::make_unique<Model>(modelPath);
        
        // Create model matrix with position and scale
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = glm::scale(modelMatrix, scale);
        
        models.push_back({std::move(newModel), modelMatrix});
        std::cout << "Model loaded: " << modelPath << " (Total models: " << models.size() << ")" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load model '" << modelPath << "': " << e.what() << std::endl;
    }
}

void RenderSystem::clearModels() {
    models.clear();
    std::cout << "All models cleared" << std::endl;
}

void RenderSystem::setModelTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale, float rotationAngle, const glm::vec3& rotationAxis) {
    if (modelIndex >= models.size()) {
        std::cerr << "Invalid model index: " << modelIndex << std::endl;
        return;
    }
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), rotationAxis);
    modelMatrix = glm::scale(modelMatrix, scale);
    
    models[modelIndex].second = modelMatrix;
}



bool RenderSystem::initialize() {

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);  // Enable multisampling
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /**shader = std::make_unique<Shader>(vertexPath.c_str(), fragmentPath.c_str());
    
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture1);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("Textures/genisys_prototype_elevator_light_emission.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2DMultisample(GL_TEXTURE_2D, 4, format, width, height, GL_TRUE);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture1" << std::endl;
        return false;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture2);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    data = stbi_load("Textures/prototype_grid_grey.png", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2DMultisample(GL_TEXTURE_2D, 4, format, width, height, GL_TRUE);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load texture2" << std::endl;
        return false;
    }
    stbi_image_free(data);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), instanceMatrices.data(), GL_DYNAMIC_DRAW);

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1); // 1 = advance per instance, not per vertex
    }

    
    shader->use();
    shader->setInt("texture1", 0);
    shader->setInt("texture2", 1);*/

    // Create model shader
    modelShader = std::make_unique<Shader>(modelVertexPath.c_str(), modelFragmentPath.c_str());

    // Create shadow shader
    shadowShader = std::make_unique<Shader>("Shaders/shadow_vertex.glsl", "Shaders/shadow_fragment.glsl");

    // Setup shadow framebuffer and depth texture
    glGenFramebuffers(1, &shadowFBO);
    glGenTextures(1, &shadowDepthMap);
    glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer is not complete!" << std::endl;
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Initialize light properties
    lightPos = glm::vec3(10.0f, 10.0f, 10.0f);
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 0.1f, 50.0f);
    //position of light in world space (used for lighting calculations in shader)
    //how to change it runtime? can it be moved with the camera?


    loadModel("models/house.obj", glm::vec3(0.f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    //model id, position, scale, rotation angle, rotation axis
    //how to speed up loading? can we load asynchronously in a separate thread and then add to the scene when ready?
    // Setup cube VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Create cube shaders
    shader = std::make_unique<Shader>("Shaders/cube_vertex.glsl", "Shaders/cube_fragment.glsl");
    std::unique_ptr<Shader> cubeShadowShader = std::make_unique<Shader>("Shaders/cube_shadow_vertex.glsl", "Shaders/cube_shadow_fragment.glsl");
    shadowShader = std::move(cubeShadowShader);

    return true;
}

void RenderSystem::render(float currentFrame, const glm::mat4& view, const glm::mat4& projection) {
    // Calculate light space matrix
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // ============ PASS 1: RENDER TO SHADOW MAP ============
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glClear(GL_DEPTH_BUFFER_BIT);  // Front face culling for shadow map

    //where is shadowmap size defined? SHADOW_WIDTH and SHADOW_HEIGHT constants in RenderSystem class
    //change light position and projection for dynamic shadows
    lightPos = glm::vec3(50.0f * cos(currentFrame), 50.0f, 50.0f * sin(currentFrame)); // Update light position if needed (e.g., for moving light)
    glm::vec3 lightTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    lightView = glm::lookAt(lightPos, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f)); // Recalculate light view matrix with new position
    lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 500.0f); // Update projection if needed (e.g., for dynamic shadow area)
    // left, right, bottom, top, near, far for orthographic projection. Adjust these values to change the shadow area and quality. For example, a smaller area can give sharper shadows but may miss objects outside the area.
    //how to render shadow map only from the perspective of the light
    // done by 
    // Render models to shadow map
    if (shadowShader && !models.empty()) {
        shadowShader->use();
        shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        for (auto& modelPair : models) {
            shadowShader->setMat4("model", modelPair.second);
            modelPair.first->draw(*shadowShader);
        }
    }


    // ============ CUBE RENDERING CODE (COMMENTED OUT FOR TESTING) ============
    //Draw large ground cube:
    /**glBindVertexArray(VAO);
    glm::mat4 groundModel = glm::mat4(1.0f);
    groundModel = glm::translate(groundModel, glm::vec3(0.0f, -5.0f, 0.0f));
    groundModel = glm::scale(groundModel, glm::vec3(100.0f, 1.0f, 100.0f));
    shadowShader->setMat4("model", groundModel);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    //
    // Draw smaller cubes:
    // for (const auto& pos : cubePositions) {
    //     glm::mat4 model = glm::mat4(1.0f);
    //     model = glm::translate(model, pos);
    //     model = glm::scale(model, glm::vec3(0.5f));
    //     shadowShader->setMat4("model", model);
    //     glDrawArrays(GL_TRIANGLES, 0, 36);
    // }*/
    // ============ PASS 2: RENDER NORMALLY WITH SHADOWS ============
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);  // Back to normal culling

    // Render models with shadows
    if (modelShader && !models.empty()) {
        modelShader->use();
        modelShader->setMat4("view", view);
        modelShader->setMat4("projection", projection);
        modelShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        // Bind shadow map to texture unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
        modelShader->setInt("shadowMap", 1);
        
        for (auto& modelPair : models) {
            modelShader->setMat4("model", modelPair.second);
            modelPair.first->draw(*modelShader);
        }
    }

    // ============ CUBE RENDERING CODE (COMMENTED OUT FOR TESTING) ============
    if (shader) {
        shader->use();
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
        shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowDepthMap);
        shader->setInt("shadowMap", 0);
        
    /**   // Draw large ground cube (grey)
        glBindVertexArray(VAO);
        glm::mat4 groundModel = glm::mat4(1.0f);
        groundModel = glm::translate(groundModel, glm::vec3(0.0f, -5.0f, 0.0f));
        groundModel = glm::scale(groundModel, glm::vec3(100.0f, 1.0f, 100.0f));
        shader->setMat4("model", groundModel);
        glUniform3f(glGetUniformLocation(shader->ID, "cubeColor"), 0.5f, 0.5f, 0.5f);
    */    glDrawArrays(GL_TRIANGLES, 0, 36);
    //     
    //     // Draw red cubes
    //     glUniform3f(glGetUniformLocation(shader->ID, "cubeColor"), 1.0f, 0.3f, 0.3f);
    //     for (const auto& pos : cubePositions) {
    //         glm::mat4 model = glm::mat4(1.0f);
    //         model = glm::translate(model, pos);
    //         model = glm::scale(model, glm::vec3(0.5f));
    //         shader->setMat4("model", model);
    //         glDrawArrays(GL_TRIANGLES, 0, 36);
    //     }
    }
}
