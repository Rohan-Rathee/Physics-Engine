#ifndef MODEL_TRANSFORM_H
#define MODEL_TRANSFORM_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <btBulletDynamicsCommon.h>


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
    

    void initializePhysicsBody(size_t modelIndex, float mass, btCollisionShape* shape, float restitution = 0.0f);
    

    void updateFrameTransforms(float deltaTime);
    bool blendAnimationsInitialized = false;
    

    btRigidBody* getPhysicsBody(size_t modelIndex);
    void applyImpulse(size_t modelIndex, const glm::vec3& impulse);
    void applyForce(size_t modelIndex, const glm::vec3& force);
    void applyTorque(size_t modelIndex, const glm::vec3& torque);
    bool isModelInAir = false;


private:
    size_t targetModelIndex = 1;
    float followDistance = 2.0f;
    float followHeight   = -0.5f;
    float smoothness     = 1.0f;
    glm::vec3 cameraOffset = glm::vec3(0.0f);
    ModelLoader* modelLoader;
    PhysicsSystem* physicsSystem;
    std::vector<PhysicsModelData> physicsModels;
    

    void applyPhysicsTransform(const PhysicsModelData& physicsModel);
};

#endif
