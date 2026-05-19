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
      modelVertexPath("Shaders/model_vertex.glsl"), modelFragmentPath("Shaders/model_fragment.glsl") {}

RenderSystem::~RenderSystem() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
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

bool RenderSystem::initialize() {

    /**glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK); 
    glFrontFace(GL_CCW);*/
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_MULTISAMPLE);  // Enable multisampling
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader = std::make_unique<Shader>(vertexPath.c_str(), fragmentPath.c_str());
    
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
    shader->setInt("texture2", 1);

    // Create model shader
    modelShader = std::make_unique<Shader>(modelVertexPath.c_str(), modelFragmentPath.c_str());

    // Load initial model
    loadModel("models/procedural_city_3.glb", glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));

    return true;
}

void RenderSystem::render(float currentFrame, const glm::mat4& view, const glm::mat4& projection) {
    glClearColor(0.44f, 0.65f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setVec3("fogColor", 0.44f, 0.65f, 1.0f);
    shader->setFloat("fogStart", 15.0f);
    shader->setFloat("fogEnd", 500.0f);

    glm::mat4 vp = projection * view;
    // Render all loaded models with model shader
    if (modelShader) {
        modelShader->use();
        modelShader->setMat4("view", view);
        modelShader->setMat4("projection", projection);
        
        for (auto& modelPair : models) {
            modelShader->setMat4("model", modelPair.second);
            modelPair.first->draw(*modelShader);
        }
    }
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