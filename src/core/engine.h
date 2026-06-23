#ifndef ENGINE_H 
#define ENGINE_H 
#include "time_manager.h" 
#include "../systems/window_system.h" 
#include "../systems/render_system.h" 
#include "../systems/input_system.h" 
#include "../systems/physics_system.h" 
#include "../utils/model_transform.h" 
#include "../scene/scene.h" 
#include "../camera.h" 
#include "../systems/camera_follow.h" 
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <memory> 
#include <vector> 
#include "../systems/imgui_system.h" 
class Character; 
extern Camera* g_camera; 
class Engine { 
private: 
    std::unique_ptr<WindowSystem> windowSystem; 
    std::unique_ptr<ImGuiSystem> imguiSystem; 
    std::unique_ptr<RenderSystem> renderSystem; 
    std::unique_ptr<InputSystem> inputSystem; 
    std::unique_ptr<TimeManager> timeManager; 
    std::unique_ptr<Scene> scene; 
    std::unique_ptr<Camera> camera; 
    std::unique_ptr<PhysicsSystem> physicsSystem; 
    std::unique_ptr<ModelTransform> modelTransform; 
    std::shared_ptr<Character> playerCharacter; 
    std::vector<std::shared_ptr<Character>> bots; 
    ThirdPersonCameraRig cameraRig; 
    unsigned int screenWidth; 
    unsigned int screenHeight; 
    bool running; 
public: 
    Engine(unsigned int width, unsigned int height, const std::string& title); 
    ~Engine(); 
    bool initialize(); 
    void run(); 
    void shutdown(); 
    std::shared_ptr<Character> spawnBot(const std::string& modelPath, const glm::vec3& spawnPosition, std::vector<glm::vec3> patrolRoute); 
    Scene* getScene() { return scene.get(); } 
    Camera* getCamera() { return camera.get(); } 
    PhysicsSystem* getPhysicsSystem() { return physicsSystem.get(); } 
}; 
#endif 
 