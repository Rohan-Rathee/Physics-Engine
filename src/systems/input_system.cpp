#include "input_system.h"

InputSystem* InputSystem::instance = nullptr;



void InputSystem::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (InputSystem::instance == nullptr) return;
    
    if (InputSystem::instance->firstMouse) {
        InputSystem::instance->lastX = static_cast<float>(xpos);
        InputSystem::instance->lastY = static_cast<float>(ypos);
        InputSystem::instance->firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - InputSystem::instance->lastX;
    float yoffset = InputSystem::instance->lastY - static_cast<float>(ypos); // reversed since y-coordinates go bottom to top

    InputSystem::instance->lastX = static_cast<float>(xpos);
    InputSystem::instance->lastY = static_cast<float>(ypos);

    InputSystem::instance->camera.ProcessMouseMovement(xoffset, yoffset);
}



void InputSystem::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (InputSystem::instance == nullptr) return;
    InputSystem::instance->camera.ProcessMouseScroll(static_cast<float>(yoffset));
}



void InputSystem::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}



InputSystem::InputSystem(GLFWwindow* w, Camera& cam, unsigned int screenWidth, unsigned int screenHeight)
    : window(w), camera(cam), deltaTime(0.0f), firstMouse(true) {
    lastX = screenWidth / 2.0f;
    lastY = screenHeight / 2.0f;
    
    InputSystem::instance = this;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}



void InputSystem::processInput() {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}
