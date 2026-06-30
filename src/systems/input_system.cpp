#include "input_system.h" 
#include "render_system.h" 
#include <imgui.h>
#include <iostream> 
#include "imgui_impl_glfw.h"
InputSystem* InputSystem::instance = nullptr; 
void InputSystem::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);


}

void InputSystem::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    if (ImGui::GetIO().WantCaptureMouse)
        return;
    if (!instance)
        return;


    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL)
        return;

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (InputSystem::instance == nullptr) return; 
     
    if (InputSystem::instance->firstMouse) { 
        InputSystem::instance->lastX = static_cast<float>(xpos); 
        InputSystem::instance->lastY = static_cast<float>(ypos); 
        InputSystem::instance->firstMouse = false; 
    } 
    float xoffset = static_cast<float>(xpos) - InputSystem::instance->lastX; 
    float yoffset = InputSystem::instance->lastY - static_cast<float>(ypos);   
    InputSystem::instance->lastX = static_cast<float>(xpos); 
    InputSystem::instance->lastY = static_cast<float>(ypos); 
    InputSystem::instance->camera.ProcessMouseMovement(xoffset, yoffset); 
} 
void InputSystem::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) { 
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (InputSystem::instance == nullptr) return; 
    InputSystem::instance->camera.ProcessMouseScroll(static_cast<float>(yoffset)); 
} 
void InputSystem::framebufferSizeCallback(GLFWwindow* window, int width, int height) { 
    glViewport(0, 0, width, height); 
    if (InputSystem::instance && InputSystem::instance->renderSystem) { 
        InputSystem::instance->renderSystem->setScreenSize(width, height); 
    } 
    InputSystem::instance->renderSystem->resizeBloomBuffers(width, height);
} 
InputSystem::InputSystem(GLFWwindow* w, Camera& cam, unsigned int screenWidth, unsigned int screenHeight) 
    : window(w), camera(cam), renderSystem(nullptr), deltaTime(0.0f), firstMouse(true) { 
    lastX = screenWidth / 2.0f; 
    lastY = screenHeight / 2.0f; 
     
    InputSystem::instance = this; 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    glfwSetCursorPosCallback(window, InputSystem::mouseCallback); 
    glfwSetScrollCallback(window, InputSystem::scrollCallback);
    glfwSetFramebufferSizeCallback(window, InputSystem::framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, InputSystem::mouseButtonCallback);

} 
void InputSystem::processInput() { 
    ImGuiIO& io = ImGui::GetIO();

    if (io.WantCaptureKeyboard || io.WantCaptureMouse)
        return;
        
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
        glfwSetWindowShouldClose(window, true); 
      
      
    static bool tabWasPressed = false; 
    bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS; 
    if (tabPressed && !tabWasPressed) 
        spectatorMode = !spectatorMode; 
    tabWasPressed = tabPressed; 
    currentInput = InputState{};   
    if (spectatorMode) 
    { 
          
          
          
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
            camera.ProcessKeyboard(FORWARD, deltaTime); 
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
            camera.ProcessKeyboard(BACKWARD, deltaTime); 
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
            camera.ProcessKeyboard(LEFT, deltaTime); 
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
            camera.ProcessKeyboard(RIGHT, deltaTime); 
    } 
    else 
    { 
          
          
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) currentInput.moveAxis.y += 1.0f; 
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) currentInput.moveAxis.y -= 1.0f; 
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) currentInput.moveAxis.x -= 1.0f; 
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) currentInput.moveAxis.x += 1.0f; 
        currentInput.jumpPressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS; 
        currentInput.firePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS; 
    } 
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) 
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 
    else 
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 
    if (renderSystem) 
    { 
        renderSystem->setBulletDebugDrawEnabled( 
            glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS 
        ); 
    } 
}