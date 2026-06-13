#ifndef MODEL_TRANSFORM_H
#define MODEL_TRANSFORM_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <btBulletDynamicsCommon.h>

// Forward declarations
class ModelLoader;
class PhysicsSystem;

struct PhysicsModelData {
    btRigidBody* rigidBody;
    btCollisionShape* collisionShape;
    size_t modelIndex;
};


class ModelTransform {
public:
    ModelTransform(ModelLoader* modelLoader, PhysicsSystem* physicsSystem);
    ~ModelTransform();
    
    void setTransform(size_t modelIndex, const glm::vec3& position, const glm::vec3& scale,
                      float rotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
    
    void updateTransform(size_t modelIndex, const glm::vec3& deltaPosition, const glm::vec3& deltaScale,
                         float deltaRotationAngle = 0.0f, const glm::vec3& rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f));
    
    // Initialize physics body for a model (reads position from modelLoader's stored transform)
    void initializePhysicsBody(size_t modelIndex, float mass, btCollisionShape* shape, float restitution = 0.0f);
    
    // Update all transforms from physics simulation
    void updateFrameTransforms(float deltaTime);
    
    // Get physics body for a model
    btRigidBody* getPhysicsBody(size_t modelIndex);
    void applyImpulse(size_t modelIndex, const glm::vec3& impulse);
    void applyForce(size_t modelIndex, const glm::vec3& force);
    void applyTorque(size_t modelIndex, const glm::vec3& torque);
    bool isModelInAir = false; // Placeholder for checking if model is in the air (for jump logic)


private:
    size_t targetModelIndex = 1;
    float followDistance = 8.0f;     // behind
    float followHeight   = 0.0f;     // height as   needed (0.0f for same level, positive for above, negative for below)
    float smoothness     = 1.0f;        // Camera lag (lower = smoother) 
    glm::vec3 cameraOffset = glm::vec3(0.0f);
    ModelLoader* modelLoader;
    PhysicsSystem* physicsSystem;
    std::vector<PhysicsModelData> physicsModels;
    
    // Helper to extract transform from rigid body
    void applyPhysicsTransform(const PhysicsModelData& physicsModel);
};

#endif
