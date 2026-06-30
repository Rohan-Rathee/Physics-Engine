#include "engine.h"
#include "../characters/character.h"
#include "../characters/human_controller.h"
#include "../characters/ai_controller.h"
#include "../characters/chess_piece_controller.h"
#include "../characters/spawner.h"
#include "../systems/camera_follow.h"
#include "../utils/model_loader.h"
#include "../utils/light_manager.h"
#include <iostream>
#include <chrono>
#include <imgui.h>

double lastTime  = glfwGetTime();
int    frameCount = 0;

Camera* g_camera = nullptr;




Engine::Engine(unsigned int width, unsigned int height, const std::string& title)
    : screenWidth(width), screenHeight(height), running(true)
{
    windowSystem  = std::make_unique<WindowSystem>(width, height, title);
    timeManager   = std::make_unique<TimeManager>();
    camera        = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    g_camera      = camera.get();
    renderSystem  = std::make_unique<RenderSystem>(
                        "Shaders/vertex.glsl", "Shaders/fragment.glsl", width, height);
    scene         = std::make_unique<Scene>("MainScene");
    physicsSystem = std::make_unique<PhysicsSystem>();
}

Engine::~Engine()
{
    shutdown();
}




bool Engine::initialize()
{
    if (!windowSystem->initialize())
    {
        std::cerr << "Failed to initialize window system\n";
        return false;
    }
    if (!physicsSystem->initialize())
    {
        std::cerr << "Failed to initialize physics system\n";
        return false;
    }
    if (!renderSystem->initialize())
    {
        std::cerr << "Failed to initialize render system\n";
        return false;
    }

    modelTransform = std::make_unique<ModelTransform>(
        renderSystem->getModelLoader(), physicsSystem.get());
    renderSystem->setModelTransformPtr(modelTransform.get());
    renderSystem->setPhysicsWorldPtr(physicsSystem->getDynamicsWorld());

    if (!renderSystem->initializeModels())
    {
        std::cerr << "Failed to initialize models\n";
        return false;
    }

    lightManager = std::make_unique<LightManager>("lights.json");
    imguiSystem  = std::make_unique<ImGuiSystem>();

    if (!imguiSystem->initialize(windowSystem->getGLFWWindow()))
    {
        std::cerr << "Failed to initialize ImGui\n";
        return false;
    }

    imguiSystem->setLightManager(lightManager.get());
    imguiSystem->setModelLoader(renderSystem->getModelLoader());

    inputSystem = std::make_unique<InputSystem>(
        windowSystem->getGLFWWindow(), *camera, screenWidth, screenHeight);
    inputSystem->setRenderSystem(renderSystem.get());

    renderSystem->setLightManager(lightManager.get());


    btRigidBody* playerBody = modelTransform->getPhysicsBody(0);
    if (playerBody)
    {
        auto humanCtrl = std::make_unique<HumanController>(inputSystem.get(), camera.get());
        playerCharacter = std::make_shared<Character>(
            0, playerBody, renderSystem->getModelLoader(), physicsSystem.get(),
            std::move(humanCtrl));
        scene->addEntity(playerCharacter);
    }
    else
    {
        std::cerr << "Engine::initialize: no rigid body at index 0 — "
                     "player character not spawned.\n";
    }




    spawnManager = std::make_unique<SpawnManager>(
        [this](const SpawnPoint& sp) -> std::shared_ptr<Character>
        {


            if (auto existing = sp.occupant.lock())
            {
                existing->respawn(sp.position);
                return existing;
            }


            return spawnBot(
                "models\\untitled1.glb",
                sp.position,
                sp.patrolPoints,
                sp.pieceType,
                sp.respawnDelay);
        });


    const float pawnX[] = { -12.0f, -10.0f, -8.0f, -6.0f, -4.0f, -2.0f, 0.0f, 2.0f,
                            4.0f,   6.0f,   8.0f, 10.0f, 12.0f };
    for (float px : pawnX)
    {
        spawnManager->addSpawnPoint({
            .position     = glm::vec3(px, 1.0f, 18.0f),
            .pieceType    = ChessPieceType::Pawn,
            .team         = 1,
            .respawnDelay = 3.0f,
            .patrolPoints = {
                glm::vec3(px, 0,  18), glm::vec3(px, 0, -18) }
        });
    }


    spawnManager->spawnAll();

    return true;
}





std::shared_ptr<Character> Engine::spawnBot(
    const std::string&     modelPath,
    const glm::vec3&       spawnPosition,
    std::vector<glm::vec3> patrolRoute,
    ChessPieceType         pieceType,
    float                  /*respawnDelay*/)
{
    ModelLoader* modelLoader = renderSystem->getModelLoader();
    modelLoader->loadModel(modelPath, spawnPosition, glm::vec3(1.0f));
    size_t botIndex = modelLoader->getModelCount() - 1;
    modelLoader->setModelAnimation(botIndex, 0);

    btCollisionShape* botShape =
        modelLoader->getModel(botIndex).model->buildCapsuleColliderFromMesh();
    glm::vec3 botScale = modelLoader->getModelScale(botIndex);
    botShape->setLocalScaling(btVector3(botScale.x, botScale.y, botScale.z));

    modelTransform->setTransform(
        botIndex, spawnPosition, glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    modelTransform->initializePhysicsBody(botIndex, 80.0f, botShape, 0.1f);
  
    btRigidBody* botBody = modelTransform->getPhysicsBody(botIndex);
    if (!botBody)   
    {
        std::cerr << "Engine::spawnBot: no rigid body at index " << botIndex << "\n";
        return nullptr;
    }   


    auto ctrl = std::make_unique<ChessPieceController>(
        pieceType, std::move(patrolRoute), speedForPiece(pieceType));
  
    if (playerCharacter)
        ctrl->setTarget(playerCharacter.get());


    auto bot = std::make_shared<Character>(
        botIndex, botBody, modelLoader, physicsSystem.get(), std::move(ctrl));

    scene->addEntity(bot);
    bots.push_back(bot);
    return bot;
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
            glfwSetWindowTitle(
                windowSystem->getGLFWWindow(),
                ("Physics Engine | FPS: " + std::to_string(frameCount)).c_str());
            frameCount = 0;
            lastTime   = currentTime;
        }


        auto t0 = std::chrono::high_resolution_clock::now();
        inputSystem->setDeltaTime(deltaTime);
        inputSystem->processInput();


        auto t1 = std::chrono::high_resolution_clock::now();
        scene->updatePrePhysics(deltaTime);
        physicsSystem->update(deltaTime);
        scene->updatePostPhysics(deltaTime);

        if (playerCharacter && !inputSystem->isSpectatorMode())
            cameraRig.update(*camera, *playerCharacter, deltaTime);


        if (spawnManager)
        {
            spawnManager->update(deltaTime);




            bots = spawnManager->getLivingCharacters();
        }


        auto t2 = std::chrono::high_resolution_clock::now();
        renderSystem->getModelLoader()->updateAnimations(deltaTime);


        auto t3 = std::chrono::high_resolution_clock::now();
        scene->update(deltaTime);


        int windowWidth, windowHeight;
        glfwGetWindowSize(windowSystem->getGLFWWindow(), &windowWidth, &windowHeight);
        screenWidth  = static_cast<unsigned int>(windowWidth);
        screenHeight = static_cast<unsigned int>(windowHeight);

        glm::mat4 projection = glm::perspective(
            glm::radians(camera->Zoom),
            (float)screenWidth / (float)screenHeight,
            0.1f, 1000000.0f);
        glm::mat4 view = camera->GetViewMatrix();


        imguiSystem->beginFrame();
        imguiSystem->renderPlayerHUD(playerCharacter.get());
        imguiSystem->renderBotHealthBars(bots, view, projection,
                                          screenWidth, screenHeight);


        renderSystem->setScreenSize(screenWidth, screenHeight);
        renderSystem->render(*camera, currentTime, view, projection);

        auto t4 = std::chrono::high_resolution_clock::now();


        float inputMs   = std::chrono::duration<float, std::milli>(t1 - t0).count();
        float physicsMs = std::chrono::duration<float, std::milli>(t2 - t1).count();
        float animMs    = std::chrono::duration<float, std::milli>(t3 - t2).count();
        float renderMs  = std::chrono::duration<float, std::milli>(t4 - t3).count();
        float totalMs   = inputMs + physicsMs + animMs + renderMs;

        ImGui::Begin("Profiler");
        ImGui::Text("Bots alive: %zu / %zu",
                    bots.size(),
                    spawnManager ? spawnManager->totalSpawnPoints() : 0u);
        ImGui::Text("Input:      %.2f ms", inputMs);
        ImGui::Text("Physics:    %.2f ms", physicsMs);
        ImGui::Text("Animation:  %.2f ms", animMs);
        ImGui::Text("Render:     %.2f ms", renderMs);
        ImGui::Text("Measured:   %.2f ms (%.0f fps)",
                    totalMs, totalMs > 0.0f ? 1000.0f / totalMs : 0.0f);
        ImGui::End();

        imguiSystem->render();


        windowSystem->swapBuffers();
        windowSystem->pollEvents();


        static bool lastF1      = false;
        static bool cursorVisible = false;
        bool f1 = glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS;
        if (f1 && !lastF1)
        {
            cursorVisible = !cursorVisible;
            glfwSetInputMode(
                windowSystem->getGLFWWindow(), GLFW_CURSOR,
                cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        lastF1 = f1;
    }
}




void Engine::shutdown()
{
    playerCharacter.reset();
    bots.clear();
    spawnManager.reset();
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




float Engine::speedForPiece(ChessPieceType t)
{
    switch (t)
    {
        case ChessPieceType::Pawn:   return 10.0f;
        case ChessPieceType::Rook:   return 10.0f;
        case ChessPieceType::Knight: return 10.0f;
        case ChessPieceType::Bishop: return 10.5f;
        case ChessPieceType::Queen:  return 10.5f;
        case ChessPieceType::King:   return 10.0f;
        default:                     return 10.0f;
    }
}