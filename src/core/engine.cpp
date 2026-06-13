#include "engine.h"
#include <iostream>

double lastTime = glfwGetTime();
int frameCount = 0;

// Global camera pointer for external access
Camera* g_camera = nullptr;

Engine::Engine(unsigned int width, unsigned int height, const std::string& title)
    : screenWidth(width), screenHeight(height), running(true) {

    windowSystem = std::make_unique<WindowSystem>(width, height, title);
    timeManager = std::make_unique<TimeManager>();
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    g_camera = camera.get();  // Set global camera pointer
    renderSystem = std::make_unique<RenderSystem>("Shaders/vertex.glsl", "Shaders/fragment.glsl", width, height);
    scene = std::make_unique<Scene>("MainScene");
    physicsSystem = std::make_unique<PhysicsSystem>();

}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {

    if (!windowSystem->initialize()) {
        std::cerr << "Failed to initialize window system" << std::endl;
        return false;
    }
    
    // Initialize physics system first
    if (!physicsSystem->initialize()) {
        std::cerr << "Failed to initialize physics system" << std::endl;
        return false;
    }
    
    // Initialize render system graphics (no models yet)
    if (!renderSystem->initialize()) {
        std::cerr << "Failed to initialize render system" << std::endl;
        return false;
    }
    
    // Create model transform with modelLoader from renderSystem and physics system
    modelTransform = std::make_unique<ModelTransform>(renderSystem->getModelLoader(), physicsSystem.get());
    renderSystem->setModelTransformPtr(modelTransform.get());
    renderSystem->setPhysicsWorldPtr(physicsSystem->getDynamicsWorld());
    
    // Now initialize models (creates physics bodies)
    if (!renderSystem->initializeModels()) {
        std::cerr << "Failed to initialize models" << std::endl;
        return false;
    }

    imguiSystem = std::make_unique<ImGuiSystem>();

    if (!imguiSystem->initialize(windowSystem->getGLFWWindow()))
    {
        std::cerr << "Failed to initialize ImGui" << std::endl;
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


        inputSystem->setDeltaTime(deltaTime);
        inputSystem->processInput();
        
        modelTransform->updateFrameTransforms(deltaTime);
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

        imguiSystem->beginFrame();

        // Update screen size in render system in case of resizing
        renderSystem->setScreenSize(screenWidth, screenHeight);
        
        glm::mat4 view = camera->GetViewMatrix();
        
        renderSystem->render(*camera, currentTime, view, projection);
        
        imguiSystem->render();

        // Swap buffers and poll events
        windowSystem->swapBuffers();
        windowSystem->pollEvents();

        //lets add a way to exit the mouse capture mode and show cursor for easier debugging
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_ENTER) == GLFW_PRESS) {
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        // Add a way to toggle the above with a single key (F1)
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS) {
            static bool cursorVisible = false;
            cursorVisible = !cursorVisible;
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
    }
}

void Engine::shutdown() {
    modelTransform.reset();
    scene.reset();
    renderSystem.reset();
    inputSystem.reset();
    windowSystem.reset();
    timeManager.reset();
    camera.reset();
    physicsSystem.reset();
    imguiSystem.reset();
}
