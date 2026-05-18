#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../camera.h"

class InputSystem {
private:
    Camera& camera;
    GLFWwindow* window;
    float deltaTime;
    float lastX, lastY;
    bool firstMouse;
    
    static InputSystem* instance;
    
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

public:
    InputSystem(GLFWwindow* w, Camera& cam, unsigned int screenWidth, unsigned int screenHeight);
    
    void processInput();
    void setDeltaTime(float dt) { deltaTime = dt; }
};

#endif
