#pragma once 
#include "../scene/entity.h" 
#include "controller.h" 
#include <memory> 
#include <glm/glm.hpp> 
class btRigidBody; 
class ModelLoader; 
class PhysicsSystem; 
  
  
  
  
class Character : public Entity 
{ 
public: 
    Character(size_t modelIndex, 
              btRigidBody* rigidBody, 
              ModelLoader* modelLoader, 
              PhysicsSystem* physicsSystem, 
              std::unique_ptr<IController> controller, 
              unsigned int runAnimIndex = 0, 
              unsigned int idleAnimIndex = 1); 
      
    void  takeDamage(float amount);
    void  heal(float amount);
    bool  isDead()     const { return health <= 0.0f; }
    float getHealth()  const { return health; }
    float getMaxHealth() const { return maxHealth; }
    void prePhysicsUpdate(float deltaTime) override; 
      
      
    void postPhysicsUpdate(float deltaTime) override; 
    size_t getModelIndex() const { return modelIndex; } 
    glm::vec3 getPosition() const; 
    glm::vec3 getForward() const; 
    glm::vec3 getUp() const; 
    bool isAirborne() const { return isInAir; } 
    IController* getController() const { return controller.get(); } 
    
    void    respawn(const glm::vec3& position);
private: 
    float health    = 100.0f;
    float maxHealth = 100.0f;
    void syncGroundedState(); 
    void applyMovement(const ControlInput& input, float deltaTime); 
    void applyFacing(const ControlInput& input, float deltaTime); 
    void syncRenderTransform(); 
    void updateAnimationBlend(); 
    size_t modelIndex; 
    btRigidBody* rigidBody; 
    ModelLoader* modelLoader; 
    PhysicsSystem* physicsSystem; 
    std::unique_ptr<IController> controller; 
    bool isInAir = true; 
    bool blendInitialized = false; 
    unsigned int runAnimIndex; 
    unsigned int idleAnimIndex; 
    float moveSpeed = 12.0f; 
    float acceleration = 12.0f; 
    float deceleration = 4.0f; 
    float jumpImpulse = 500.0f; 
    float turnSpeed = 50.0f; 
}; 
