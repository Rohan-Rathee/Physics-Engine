#include "engine.h"
#include <iostream>
#include <chrono>
#include <imgui.h>

double lastTime = glfwGetTime();
int frameCount = 0;
Camera* g_camera = nullptr;

Engine::Engine(unsigned int width, unsigned int height, const std::string& title)
    : screenWidth(width), screenHeight(height), running(true)
{
    windowSystem  = std::make_unique<WindowSystem>(width, height, title);
    timeManager   = std::make_unique<TimeManager>();
    camera        = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    g_camera      = camera.get();
    renderSystem  = std::make_unique<RenderSystem>("Shaders/vertex.glsl", "Shaders/fragment.glsl", width, height);
    scene         = std::make_unique<Scene>("MainScene");
    physicsSystem = std::make_unique<PhysicsSystem>();
}

Engine::~Engine() { shutdown(); }

bool Engine::initialize(IGame* gamePtr)
{
    game = gamePtr;

    if (!windowSystem->initialize())  { std::cerr << "Failed to initialize window system\n"; return false; }
    if (!physicsSystem->initialize()) { std::cerr << "Failed to initialize physics system\n"; return false; }
    if (!renderSystem->initialize())  { std::cerr << "Failed to initialize render system\n"; return false; }

    {
        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(windowSystem->getGLFWWindow(), &fbWidth, &fbHeight);
        if (fbWidth > 0 && fbHeight > 0)
        {
            screenWidth  = static_cast<unsigned int>(fbWidth);
            screenHeight = static_cast<unsigned int>(fbHeight);
            renderSystem->setScreenSize(screenWidth, screenHeight);
            renderSystem->resizeBloomBuffers(fbWidth, fbHeight);
        }
    }

    modelTransform = std::make_unique<ModelTransform>(renderSystem->getModelLoader(), physicsSystem.get());
    renderSystem->setModelTransformPtr(modelTransform.get());
    renderSystem->setPhysicsWorldPtr(physicsSystem->getDynamicsWorld());

    if (!renderSystem->initializeModels()) { std::cerr << "Failed to initialize models\n"; return false; }

    lightManager = std::make_unique<LightManager>("lights.json");
    imguiSystem  = std::make_unique<ImGuiSystem>();
    if (!imguiSystem->initialize(windowSystem->getGLFWWindow())) { std::cerr << "Failed to initialize ImGui\n"; return false; }

    imguiSystem->setLightManager(lightManager.get());
    imguiSystem->setModelLoader(renderSystem->getModelLoader());
    renderSystem->setLightManager(lightManager.get());

    inputSystem = std::make_unique<InputSystem>(windowSystem->getGLFWWindow(), *camera, screenWidth, screenHeight);
    inputSystem->setRenderSystem(renderSystem.get());

    // Everything game-specific — player creation, spawn points, chess pieces — happens here.
    if (!game || !game->onInitialize(*this))
    {
        std::cerr << "Engine::initialize: game initialization failed\n";
        return false;
    }

    return true;
}

void Engine::run()
{
    while (!windowSystem->shouldClose() && running)
    {
        timeManager->update();
        float deltaTime   = timeManager->getDeltaTime();
        float currentTime = timeManager->getCurrentTime();
        frameCount++;
        if (currentTime - lastTime >= 1.0)
        {
            glfwSetWindowTitle(windowSystem->getGLFWWindow(),
                ("Engine | FPS: " + std::to_string(frameCount)).c_str());
            frameCount = 0;
            lastTime = currentTime;
        }

        auto t0 = std::chrono::high_resolution_clock::now();
        inputSystem->setDeltaTime(deltaTime);
        inputSystem->processInput();

        auto t1 = std::chrono::high_resolution_clock::now();
        scene->updatePrePhysics(deltaTime);
        physicsSystem->update(deltaTime);
        scene->updatePostPhysics(deltaTime);

        Character* camTarget = game ? game->getCameraTarget() : nullptr;
        if (camTarget && !inputSystem->isSpectatorMode())
            cameraRig.update(*camera, *camTarget, deltaTime);

        if (game) game->onUpdate(deltaTime);

        auto t2 = std::chrono::high_resolution_clock::now();
        renderSystem->getModelLoader()->updateAnimations(deltaTime);

        auto t3 = std::chrono::high_resolution_clock::now();
        scene->update(deltaTime);

        int windowWidth, windowHeight;
        glfwGetWindowSize(windowSystem->getGLFWWindow(), &windowWidth, &windowHeight);
        screenWidth  = static_cast<unsigned int>(windowWidth);
        screenHeight = static_cast<unsigned int>(windowHeight);
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom),
            (float)screenWidth / (float)screenHeight, 0.1f, 1000000.0f);
        glm::mat4 view = camera->GetViewMatrix();

        imguiSystem->beginFrame();
        if (game) game->onRenderUI(); // player HUD, bot health bars, "bots alive" — all game-owned now

        renderSystem->setScreenSize(screenWidth, screenHeight);
        renderSystem->render(*camera, currentTime, view, projection);
        auto t4 = std::chrono::high_resolution_clock::now();

        float inputMs   = std::chrono::duration<float, std::milli>(t1 - t0).count();
        float physicsMs = std::chrono::duration<float, std::milli>(t2 - t1).count();
        float animMs    = std::chrono::duration<float, std::milli>(t3 - t2).count();
        float renderMs  = std::chrono::duration<float, std::milli>(t4 - t3).count();
        float totalMs   = inputMs + physicsMs + animMs + renderMs;

        ImGui::Begin("Engine Profiler"); // purely engine-level now, no gameplay counters
        ImGui::Text("Input:      %.2f ms", inputMs);
        ImGui::Text("Physics:    %.2f ms", physicsMs);
        ImGui::Text("Animation:  %.2f ms", animMs);
        ImGui::Text("Render:     %.2f ms", renderMs);
        ImGui::Text("Measured:   %.2f ms (%.0f fps)", totalMs, totalMs > 0.0f ? 1000.0f / totalMs : 0.0f);
        ImGui::End();
        imguiSystem->render();

        windowSystem->swapBuffers();
        windowSystem->pollEvents();

        static bool lastF1 = false, cursorVisible = false;
        bool f1 = glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS;
        if (f1 && !lastF1)
        {
            cursorVisible = !cursorVisible;
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR,
                cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        lastF1 = f1;
    }
}

void Engine::shutdown()
{
    if (game) game->onShutdown();
    scene.reset();
    modelTransform.reset();
    renderSystem.reset();
    inputSystem.reset();
    windowSystem.reset();
    timeManager.reset();
    camera.reset();
    physicsSystem.reset();
    imguiSystem.reset();
}