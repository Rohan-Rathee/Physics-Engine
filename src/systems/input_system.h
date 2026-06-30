#ifndef INPUT_SYSTEM_H 
#define INPUT_SYSTEM_H 
#include <glad/glad.h> 
#include <GLFW/glfw3.h> 
#include "../camera.h" 
#include <glm/glm.hpp> 
struct InputState { 
    glm::vec2 moveAxis{0.0f}; 
    bool jumpPressed = false; 
    bool firePressed = false; 
}; 
class RenderSystem;    
class InputSystem { 
private: 
    Camera& camera; 
    RenderSystem* renderSystem; 
    GLFWwindow* window; 
    float deltaTime; 
    InputState currentInput; 
    bool spectatorMode = false; 
    bool firstMouse; 
    bool mKeyPressed = false; 
     
    static InputSystem* instance; 
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos); 
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset); 
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height); 
public: 
    InputState getInputState() const { return currentInput; } 
    float lastX, lastY; 
    InputSystem(GLFWwindow* w, Camera& cam, unsigned int screenWidth, unsigned int screenHeight); 
     
    void setRenderSystem(RenderSystem* rs) { renderSystem = rs; } 
    void processInput(); 
    void setDeltaTime(float dt) { deltaTime = dt; } 
    bool isSpectatorMode() const { return spectatorMode; }
}; 
#endif 
