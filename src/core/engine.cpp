#include "engine.h" 
#include "../characters/character.h" 
#include "../characters/human_controller.h" 
#include "../characters/ai_controller.h" 
#include "../systems/camera_follow.h" 
#include "../utils/model_loader.h" 
#include <iostream> 
#include <chrono> 
#include <imgui.h> 
double lastTime = glfwGetTime(); 
int frameCount = 0; 
  
Camera *g_camera = nullptr; 
Engine::Engine(unsigned int width, unsigned int height, const std::string &title) 
    : screenWidth(width), screenHeight(height), running(true) 
{ 
    windowSystem = std::make_unique<WindowSystem>(width, height, title); 
    timeManager = std::make_unique<TimeManager>(); 
    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f)); 
    g_camera = camera.get();   
    renderSystem = std::make_unique<RenderSystem>("Shaders/vertex.glsl", "Shaders/fragment.glsl", width, height); 
    scene = std::make_unique<Scene>("MainScene"); 
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
        std::cerr << "Failed to initialize window system" << std::endl; 
        return false; 
    } 
      
    if (!physicsSystem->initialize()) 
    { 
        std::cerr << "Failed to initialize physics system" << std::endl; 
        return false; 
    } 
      
    if (!renderSystem->initialize()) 
    { 
        std::cerr << "Failed to initialize render system" << std::endl; 
        return false; 
    } 
      
    modelTransform = std::make_unique<ModelTransform>(renderSystem->getModelLoader(), physicsSystem.get()); 
    renderSystem->setModelTransformPtr(modelTransform.get()); 
    renderSystem->setPhysicsWorldPtr(physicsSystem->getDynamicsWorld()); 
      
    if (!renderSystem->initializeModels()) 
    { 
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
      
      
      
      
    btRigidBody *playerBody = modelTransform->getPhysicsBody(0); 
    if (playerBody) 
    { 
        auto humanController = std::make_unique<HumanController>(inputSystem.get(), camera.get()); 
        playerCharacter = std::make_shared<Character>( 
            0, playerBody, renderSystem->getModelLoader(), physicsSystem.get(), 
            std::move(humanController)); 
        scene->addEntity(playerCharacter); 
    } 
    else 
    { 
        std::cerr << "Engine::initialize: no rigid body found at model index 0 - " 
                     "player character was not spawned." 
                  << std::endl; 
    } 
      
     
    for (int i = 0; i < 2; i++) 
    { 
        spawnBot("models\\untitled1.glb", glm::vec3(0.0f + i, 10.0f, 5.0f), {glm::vec3(5.0f + i * 2.0f, 0.0f, 5.0f), glm::vec3(-5.0f + i * 2.0f, 0.0f, 5.0f), glm::vec3(-5.0f + i * 2.0f, 0.0f, -5.0f), glm::vec3(5.0f + i * 2.0f, 0.0f, -5.0f)}); 
         
     
    } 
    return true; 
} 
std::shared_ptr<Character> Engine::spawnBot(const std::string &modelPath, const glm::vec3 &spawnPosition, std::vector<glm::vec3> patrolRoute) 
{ 
    ModelLoader *modelLoader = renderSystem->getModelLoader(); 
    modelLoader->loadModel(modelPath, spawnPosition, glm::vec3(1.0f)); 
    size_t botIndex = modelLoader->getModelCount() - 1; 
    modelLoader->setModelAnimation(botIndex, 0); 
    btCollisionShape *botShape = 
        modelLoader->getModel(botIndex)   
            .model               
            ->buildCapsuleColliderFromMesh();        
    glm::vec3 botScale = modelLoader->getModelScale(botIndex);      
    botShape->setLocalScaling(btVector3(botScale.x, botScale.y, botScale.z)); 
    modelTransform->setTransform(botIndex, spawnPosition, glm::vec3(1.0f), 0.0f, glm::vec3(1.0f, 0.0f, 0.0f)); 
    modelTransform->initializePhysicsBody(botIndex, 80.0f, botShape, 0.1f); 
    btRigidBody *botBody = modelTransform->getPhysicsBody(botIndex); 
    if (!botBody) 
    { 
        std::cerr << "Engine::spawnBot: failed to create rigid body for bot at index " << botIndex << std::endl; 
        return nullptr; 
    } 
    auto aiController = std::make_unique<AIController>(std::move(patrolRoute)); 
    auto bot = std::make_shared<Character>(botIndex, botBody, modelLoader, physicsSystem.get(), std::move(aiController)); 
     
    scene->addEntity(bot); 
    bots.push_back(bot); 
    return bot; 
} 
void Engine::run() 
{ 
    while (!windowSystem->shouldClose() && running) 
    { 
          
        timeManager->update(); 
        float deltaTime = timeManager->getDeltaTime(); 
        float currentTime = timeManager->getCurrentTime(); 
          
        frameCount++; 
        if (currentTime - lastTime >= 1.0) 
        { 
            std::string title = "Physics Engine | FPS: " + std::to_string(frameCount); 
            glfwSetWindowTitle(windowSystem->getGLFWWindow(), title.c_str()); 
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
        if (playerCharacter) 
            cameraRig.update(*camera, *playerCharacter, deltaTime); 
        auto t2 = std::chrono::high_resolution_clock::now(); 
        renderSystem->getModelLoader()->updateAnimations(deltaTime); 
        auto t3 = std::chrono::high_resolution_clock::now(); 
          
        scene->update(deltaTime); 
          
          
        int windowWidth, windowHeight; 
        glfwGetWindowSize(windowSystem->getGLFWWindow(), &windowWidth, &windowHeight); 
        screenWidth = static_cast<unsigned int>(windowWidth); 
        screenHeight = static_cast<unsigned int>(windowHeight); 
        glm::mat4 projection = glm::perspective( 
            glm::radians(camera->Zoom), 
            (float)screenWidth / (float)screenHeight, 
            0.1f, 
            1000000.0f); 
        imguiSystem->beginFrame(); 
          
        renderSystem->setScreenSize(screenWidth, screenHeight); 
        glm::mat4 view = camera->GetViewMatrix(); 
        renderSystem->render(*camera, currentTime, view, projection); 
        auto t4 = std::chrono::high_resolution_clock::now(); 
        float inputMs   = std::chrono::duration<float, std::milli>(t1 - t0).count(); 
        float physicsMs = std::chrono::duration<float, std::milli>(t2 - t1).count(); 
        float animMs    = std::chrono::duration<float, std::milli>(t3 - t2).count(); 
        float renderMs  = std::chrono::duration<float, std::milli>(t4 - t3).count(); 
        float totalMs   = inputMs + physicsMs + animMs + renderMs; 
          
          
          
        ImGui::Begin("Profiler"); 
        ImGui::Text("Bots:      %zu", bots.size()); 
        ImGui::Text("Input:     %.2f ms", inputMs); 
        ImGui::Text("Physics:   %.2f ms", physicsMs); 
        ImGui::Text("Animation: %.2f ms", animMs); 
        ImGui::Text("Render:    %.2f ms", renderMs); 
        ImGui::Text("Measured:  %.2f ms (%.0f fps)", totalMs, totalMs > 0.0f ? 1000.0f / totalMs : 0.0f); 
        ImGui::End(); 
        imguiSystem->render(); 
          
        windowSystem->swapBuffers(); 
        windowSystem->pollEvents(); 
          
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_BACKSPACE) == GLFW_PRESS) 
        { 
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
        } 
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_ENTER) == GLFW_PRESS) 
        { 
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
        } 
          
        if (glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS) 
        { 
            static bool cursorVisible = false; 
            cursorVisible = !cursorVisible; 
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR, cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED); 
        } 
    } 
} 
void Engine::shutdown() 
{ 
    playerCharacter.reset(); 
    bots.clear(); 
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