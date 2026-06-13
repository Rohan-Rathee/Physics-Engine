#include "engine.h"
#include <iostream>

double lastTime = glfwGetTime();
int frameCount = 0;


Camera* g_camera = nullptr;

Engine::Engine(unsigned int width, unsigned int height, const std::string& title)
    : screenWidth(width), screenHeight(height), running(true) {

    windowSystem = std::make_unique<WindowSystem>(width, height, title);
    timeManager = std::make_unique<TimeManager>();
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    g_camera = camera.get();
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
    

    if (!physicsSystem->initialize()) {
        std::cerr << "Failed to initialize physics system" << std::endl;
        return false;
    }
    

    if (!renderSystem->initialize()) {
        std::cerr << "Failed to initialize render system" << std::endl;
        return false;
    }
    

    modelTransform = std::make_unique<ModelTransform>(renderSystem->getModelLoader(), physicsSystem.get());
    renderSystem->setModelTransformPtr(modelTransform.get());
    renderSystem->setPhysicsWorldPtr(physicsSystem->getDynamicsWorld());
    

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
    

    inputSystem = std::make_unique<InputSystem>(windowSystem->getGLFWWindow(), *camera, screenWidth, screenHeight);
    inputSystem->setRenderSystem(renderSystem.get());
    
    return true;
}

void Engine::run() {
    while (!windowSystem->shouldClose() && running) {


        timeManager->update();
        float deltaTime = timeManager->getDeltaTime();
        float currentTime = timeManager->getCurrentTime();
        

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

        scene->update(deltaTime);
        


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


        renderSystem->setScreenSize(screenWidth, screenHeight);
        
        glm::mat4 view = camera->GetViewMatrix();
        
        renderSystem->render(*camera, currentTime, view, projection);
        
        imguiSystem->render();


        windowSystem->swapBuffers();
        windowSystem->pollEvents();


        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_ENTER) == GLFW_PRESS) {
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

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
