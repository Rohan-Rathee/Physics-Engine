/**
 * @file engine.cpp
 * @brief Implements the core Engine class responsible for initializing,
 * updating, rendering, and shutting down the game engine.
 *
 * basically the backbone of the entire engine and is the conly file called from main.cpp.
 *
 *
 * first it takes ownership og all systmes and managers is requires, and initializes them in doing so. 
 * after the first round of initializations, it enters the secondary initialization phase, where it sets up the
 * model transform, light manager, imgui , input and render system  each of which require their own .get() calls to own the sub imports/ subsystems.
 * (spawn bots ode is also handeled here, currently they are hardcoded to just patrol and then are externally set to always target the player character, which is also hardcoded to spawn at a specific location)
 * model sets are also hardcoded to be the same for all chess pieces, but this can be changed in the future to have different models for each piece type.
 * 
 * spawn bots doesnt really belong here, but it is a good place to put it for now, as it requires access to the scene, render system, and physics system, which are all initialized here, and i am currently only working on the bot control
 * 
 * main loop first steps time, then adds fps, input stufff(mouse and keyboard to input system), then steps physics, then steps animation, then renders the scene, then renders imgui, then swaps buffers and polls events. IN THAT ORDER, physics kept before render cuz this is a loop 
 * everything in the loop is a circle not a line, after polling, it does input stuff instantly, and there is no start adn end to main loop (maybe a start but definitely no end, it is a circle, not a line but must begin somewhere)
 * in hindsight this aint that big a deal or a new concept, but it is a good way to not have to worry too much about rendering after physcis or physics after rendering messing stuff up
 * 
 * 
 * then basic shutdown. plain and simple
 * 
 * @author Rohan Rathee
 * @date 2026
 */


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
#include "../utils/light_manager.h"
#include "../characters/spawner.h"
#include "../characters/chess_piece_controller.h"

class Character; 
extern Camera* g_camera; 
class Engine { 
private: 

    std::unique_ptr<SpawnManager> spawnManager;
    std::unique_ptr<WindowSystem> windowSystem; 
    std::unique_ptr<LightManager> lightManager;
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

    float       speedForPiece(ChessPieceType t);
    std::string modelForPiece(ChessPieceType t);

public: 

    Engine(unsigned int width, unsigned int height, const std::string& title); 
    ~Engine(); 

    bool initialize(); 
    void run(); 
    void shutdown(); 

    std::shared_ptr<Character> spawnBot(
    const std::string&     modelPath,
    const glm::vec3&       spawnPosition,
    std::vector<glm::vec3> patrolRoute,
    ChessPieceType         pieceType    = ChessPieceType::Pawn,
    float                  respawnDelay = 5.0f);


    Scene* getScene() { return scene.get(); } 
    Camera* getCamera() { return camera.get(); } 

    PhysicsSystem* getPhysicsSystem() { return physicsSystem.get(); } 
}; 
#endif