#ifndef ENGINE_H
#define ENGINE_H

#include "time_manager.h"
#include "../systems/window_system.h"
#include "../systems/render_system.h"
#include "../systems/input_system.h"
#include "../scene/scene.h"
#include "../camera.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Engine {
private:
    std::unique_ptr<WindowSystem> windowSystem;
    std::unique_ptr<RenderSystem> renderSystem;
    std::unique_ptr<InputSystem> inputSystem;
    std::unique_ptr<TimeManager> timeManager;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Camera> camera;
    
    unsigned int screenWidth;
    unsigned int screenHeight;
    bool running;

public:
    Engine(unsigned int width, unsigned int height, const std::string& title);
    ~Engine();
    
    bool initialize();
    void run();
    void shutdown();
    
    Scene* getScene() { return scene.get(); }
    Camera* getCamera() { return camera.get(); }
};

#endif
