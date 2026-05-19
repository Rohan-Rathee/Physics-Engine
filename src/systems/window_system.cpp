#include "window_system.h"
#include <iostream>

WindowSystem::WindowSystem(unsigned int w, unsigned int h, const std::string& t)
    : window(nullptr), width(w), height(h), title(t) {}

WindowSystem::~WindowSystem() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool WindowSystem::initialize() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);  // Enable 4x MSAA

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  // Enable multisampling
    return true;
}

bool WindowSystem::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void WindowSystem::swapBuffers() {
    glfwSwapBuffers(window);
}

void WindowSystem::pollEvents() {
    glfwPollEvents();
}

void WindowSystem::close() {
    glfwSetWindowShouldClose(window, true);
}
