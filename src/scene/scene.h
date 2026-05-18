#ifndef SCENE_H
#define SCENE_H

#include "entity.h"
#include <vector>
#include <memory>

class Scene {
private:
    std::vector<std::shared_ptr<Entity>> entities;
    std::string name;

public:
    Scene(const std::string& sceneName = "Scene");
    
    std::shared_ptr<Entity> addEntity(std::shared_ptr<Entity> entity);
    void removeEntity(std::shared_ptr<Entity> entity);
    
    const std::vector<std::shared_ptr<Entity>>& getEntities() const { return entities; }
    std::vector<std::shared_ptr<Entity>>& getEntities() { return entities; }
    
    void update(float deltaTime);
    void render();
    
    std::string getName() const { return name; }
};

#endif
