/**
 * @file scene.h and entity.h
 * @brief integrated wiht the scene and entity management for the game engine. mainly pre and post physics animation and rendering helper(not exactly sure what to call it, not a helper and not a full on system, but works as is)
 *
 * is simple and plan to keep is simple
 */
 #ifndef SCENE_H 
#define SCENE_H 
#include "entity.h" 
#include <vector> 
#include <memory> 
#include <string> 
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
     
    void updatePrePhysics(float deltaTime);  
    void updatePostPhysics(float deltaTime);  
    void update(float deltaTime); 
    void render(); 
     
    std::string getName() const { return name; } 
}; 
#endif 
