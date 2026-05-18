#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "../shader.h"
#include <string>
#include <memory>

class RenderSystem {
private:
    std::unique_ptr<Shader> shader;
    unsigned int instanceVBO;
    unsigned int VAO, VBO;
    unsigned int texture1, texture2;
    unsigned int screenWidth, screenHeight;
    std::string vertexPath, fragmentPath;
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
};


#endif
