#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../camera.h"

class RenderSystem;  // Forward declaration

class InputSystem {
private:

    Camera& camera;
    RenderSystem* renderSystem;
    GLFWwindow* window;
    float deltaTime;

    bool firstMouse;
    bool mKeyPressed = false;
    
    static InputSystem* instance;
    
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

public:
    float lastX, lastY;
    InputSystem(GLFWwindow* w, Camera& cam, unsigned int screenWidth, unsigned int screenHeight);
    
    void setRenderSystem(RenderSystem* rs) { renderSystem = rs; }
    void processInput();
    void setDeltaTime(float dt) { deltaTime = dt; }
};

#endif
