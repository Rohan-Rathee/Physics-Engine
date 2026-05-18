#include "time_manager.h"
#include <GLFW/glfw3.h>
#include <cmath>

TimeManager::TimeManager() : deltaTime(0.0f), lastFrame(0.0f), currentFrame(0.0f) {}

void TimeManager::update() {
    currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

float TimeManager::getFPS() const {
    if (deltaTime == 0.0f) return 0.0f;
    return 1.0f / deltaTime;
}
