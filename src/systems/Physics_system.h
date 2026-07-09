/**
 * @file physics_system.h
 * @brief Wrapper around the Bullet Physics simulation world.
 *
 * a joint between the game engine and the Bullet Physics library. kinda a wrapper around the lib to make it talk to model transform a bit better.
 * 
 */
#pragma once 
#include <memory> 
#include <btBulletDynamicsCommon.h> 
#include <BulletDynamics/Dynamics/btRigidBody.h> 
#include <vector> 
#include <glm/glm.hpp> 
class PhysicsSystem { 
private: 
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig; 
    std::unique_ptr<btCollisionDispatcher> dispatcher; 
    std::unique_ptr<btDbvtBroadphase> broadphase; 
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver; 
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld; 
     
    glm::vec3 gravity; 
    float accumulator = 0.0f; 
    float fixedStep = 1.0f / 60.0f;   
public: 
    PhysicsSystem(); 
    ~PhysicsSystem(); 
     
    bool initialize(const glm::vec3& gravityVec = glm::vec3(0, -9.81f, 0)); 
    btDynamicsWorld* getDynamicsWorld() const { return dynamicsWorld.get(); } 
    void update(float deltaTime); 
    void shutdown(); 
     
      
    btRigidBody* createRigidBody(float mass, btCollisionShape* shape,  
                                  const glm::vec3& position, float restitution = 0.0f); 
     
      
    void removeRigidBody(btRigidBody* body); 
     
      
    btDynamicsWorld* getDynamicsWorld() { return dynamicsWorld.get(); } 
    glm::vec3 getGravity() const { return gravity; } 
    void setGravity(const glm::vec3& g); 
}; 
