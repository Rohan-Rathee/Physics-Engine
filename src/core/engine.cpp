#include "engine.h"
#include <iostream>

double lastTime = glfwGetTime();
int frameCount = 0;

Engine::Engine(unsigned int width, unsigned int height, const std::string& title)
    : screenWidth(width), screenHeight(height), running(true) {

    windowSystem = std::make_unique<WindowSystem>(width, height, title);
    timeManager = std::make_unique<TimeManager>();
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    renderSystem = std::make_unique<RenderSystem>("Shaders/vertex.glsl", "Shaders/fragment.glsl", width, height);
    scene = std::make_unique<Scene>("MainScene");

}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {

    if (!windowSystem->initialize()) {
        std::cerr << "Failed to initialize window system" << std::endl;
        return false;
    }
    
    if (!renderSystem->initialize()) {
        std::cerr << "Failed to initialize render system" << std::endl;
        return false;
    }
    
    //for camera input handling
    inputSystem = std::make_unique<InputSystem>(windowSystem->getGLFWWindow(), *camera, screenWidth, screenHeight);
    inputSystem->setRenderSystem(renderSystem.get());
    
    return true;
}

void Engine::run() {
    while (!windowSystem->shouldClose() && running) {

        // Update timing
        timeManager->update();
        float deltaTime = timeManager->getDeltaTime();
        float currentTime = timeManager->getCurrentTime();
        
        // Update FPS counter
        frameCount++;       
        if (currentTime - lastTime >= 1.0) {
            std::string title = "Physics Engine | FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(windowSystem->getGLFWWindow(), title.c_str());
            frameCount = 0;
            lastTime = currentTime;
        }


        // Handle input
        inputSystem->setDeltaTime(deltaTime);
        inputSystem->processInput();
        

        // Update scene
        scene->update(deltaTime);
        
        // Render
        // Update screen size from actual window size
        int windowWidth, windowHeight;
        glfwGetWindowSize(windowSystem->getGLFWWindow(), &windowWidth, &windowHeight);
        screenWidth = static_cast<unsigned int>(windowWidth);
        screenHeight = static_cast<unsigned int>(windowHeight);
        glm::mat4 projection = glm::perspective(
            glm::radians(camera->Zoom),
            (float)screenWidth / (float)screenHeight,
            0.1f,
            1000000.0f
        );

        // Update screen size in render system in case of resizing
        renderSystem->setScreenSize(screenWidth, screenHeight);
        
        glm::mat4 view = camera->GetViewMatrix();
        
        renderSystem->render(*camera, currentTime, view, projection);
        
        // Swap buffers and poll events
        windowSystem->swapBuffers();
        windowSystem->pollEvents();
    }
}

void Engine::shutdown() {
    scene.reset();
    renderSystem.reset();
    inputSystem.reset();
    windowSystem.reset();
    timeManager.reset();
    camera.reset();
}
