/**
 * @file model_transform.h
 * @brief Manages model transforms and their associated physics bodies.
 *
 * helper to ModelLoader and PhysicsSystem, this class manages the transforms of models in the scene, including position, scale, and rotation.
 *
 * hr 60 update: was about to become the new all in one handles too mch class but saved it by offloading most stuff
 * 
 * Also handles:
 * - Physics body creation. (new as of hr 50)
 * - Force, impulse, and torque application.
 * - Transform synchronization between rendering and physics.
 * - Entire character movement and animation. REMOVED SHIFTED TO CHARACTER CLASS 
 *
 * ------------------------
 * This class is the bridge between graphics and physics.
 * ------------------------
 */

 #pragma once
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class ModelLoader;
class PhysicsSystem;

struct PhysicsModelData {
    btRigidBody *rigidBody;
    btCollisionShape *collisionShape;
    size_t modelIndex;
};

class ModelTransform {
public:
    ModelTransform(ModelLoader *modelLoader, PhysicsSystem *physicsSystem);
    ~ModelTransform();

    void setTransform(size_t modelIndex, const glm::vec3 &position, const glm::vec3 &scale, float rotationAngle = 0.0f, const glm::vec3 &rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
    void updateTransform(size_t modelIndex, const glm::vec3 &deltaPosition, const glm::vec3 &deltaScale, float deltaRotationAngle = 0.0f, const glm::vec3 &rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));

    void initializePhysicsBody(size_t modelIndex, float mass, btCollisionShape *shape, float restitution = 0.0f);

    bool blendAnimationsInitialized = false;

    btRigidBody *getPhysicsBody(size_t modelIndex);
    void applyImpulse(size_t modelIndex, const glm::vec3 &impulse);
    void applyForce(size_t modelIndex, const glm::vec3 &force);
    void applyTorque(size_t modelIndex, const glm::vec3 &torque);

private:
    glm::vec3 cameraOffset = glm::vec3(0.0f);
    ModelLoader *modelLoader;
    PhysicsSystem *physicsSystem;
    std::vector<PhysicsModelData> physicsModels;

    void applyPhysicsTransform(const PhysicsModelData &physicsModel);
};

