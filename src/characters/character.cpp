#include "character.h" 
#include "../utils/model_loader.h" 
#include "../systems/physics_system.h"     
#include <btBulletDynamicsCommon.h> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/quaternion.hpp> 
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/quaternion.hpp> 
Character::Character(size_t modelIndex, 
                      btRigidBody* rigidBody, 
                      ModelLoader* modelLoader, 
                      PhysicsSystem* physicsSystem, 
                      std::unique_ptr<IController> controller, 
                      unsigned int runAnimIndex, 
                      unsigned int idleAnimIndex) 
    : modelIndex(modelIndex), 
      rigidBody(rigidBody), 
      modelLoader(modelLoader), 
      physicsSystem(physicsSystem), 
      controller(std::move(controller)), 
      runAnimIndex(runAnimIndex), 
      idleAnimIndex(idleAnimIndex) 
{ 
} 
glm::vec3 Character::getPosition() const 
{ 
    if (!rigidBody) return glm::vec3(0.0f); 
    btVector3 p = rigidBody->getWorldTransform().getOrigin(); 
    return glm::vec3(p.x(), p.y(), p.z()); 
} 
glm::vec3 Character::getForward() const 
{ 
    if (!rigidBody) return glm::vec3(0.0f, 0.0f, -1.0f); 
    btQuaternion r = rigidBody->getWorldTransform().getRotation(); 
    glm::quat rot(r.w(), r.x(), r.y(), r.z()); 
    return glm::normalize(rot * glm::vec3(-1.0f, 0.0f, 0.0f)); 
} 
glm::vec3 Character::getUp() const 
{ 
    if (!rigidBody) return glm::vec3(0.0f, 1.0f, 0.0f); 
    btQuaternion r = rigidBody->getWorldTransform().getRotation(); 
    glm::quat rot(r.w(), r.x(), r.y(), r.z()); 
      
    return glm::normalize(rot * glm::vec3(0.0f, -1.0f, 0.0f)); 
} 
void Character::prePhysicsUpdate(float deltaTime) 
{ 
    if (!rigidBody || !controller) 
        return; 
    ControlInput input = controller->getInput(deltaTime, getPosition()); 
    syncGroundedState(); 
    applyMovement(input, deltaTime); 
    applyFacing(input, deltaTime); 
} 
void Character::postPhysicsUpdate(float deltaTime) 
{ 
    if (!rigidBody) 
        return; 
    syncRenderTransform(); 
    updateAnimationBlend(); 
} 
void Character::syncGroundedState() 
{ 
    if (!physicsSystem) { isInAir = true; return; } 
    btVector3 startPos = rigidBody->getWorldTransform().getOrigin(); 
    btVector3 endPos = startPos - btVector3(0, 1.0f, 0); 
    btCollisionWorld::ClosestRayResultCallback rayCallback(startPos, endPos); 
    physicsSystem->getDynamicsWorld()->rayTest(startPos, endPos, rayCallback); 
    isInAir = !rayCallback.hasHit(); 
} 
void Character::applyMovement(const ControlInput& input, float deltaTime) 
{ 
    if (input.jumpPressed && !isInAir) 
    { 
        rigidBody->activate(true); 
        rigidBody->applyCentralImpulse(btVector3(0, jumpImpulse, 0)); 
        isInAir = true; 
    } 
    btVector3 vel = rigidBody->getLinearVelocity(); 
    if (!isInAir) 
    { 
        glm::vec2 currentVel(vel.x(), vel.z()); 
        glm::vec2 targetVel(0.0f); 
        if (glm::length(input.moveDir) > 0.0001f) 
            targetVel = glm::vec2(input.moveDir.x, input.moveDir.z) * moveSpeed; 
        float blend = glm::length(targetVel) > glm::length(currentVel) ? acceleration : deceleration; 
        currentVel = glm::mix(currentVel, targetVel, blend * deltaTime); 
        rigidBody->setLinearVelocity(btVector3(currentVel.x, vel.y(), currentVel.y)); 
    } 
    else if (glm::length(input.moveDir) > 0.0001f) 
    { 
        rigidBody->activate(true); 
        rigidBody->applyCentralForce(btVector3(input.moveDir.x, input.moveDir.y, input.moveDir.z) * 200.0f); 
    } 
} 
void Character::applyFacing(const ControlInput& input, float deltaTime) 
{ 
      
      
      
    glm::vec3 desiredForward = input.aimDir; 
    desiredForward.y = 0.0f; 
    if (glm::length(desiredForward) <= 0.0001f) 
        return; 
    desiredForward = glm::normalize(desiredForward); 
    float yaw = -atan2(desiredForward.z, desiredForward.x) + glm::radians(180.0f); 
    btQuaternion targetRotation(btVector3(0, 1, 0), yaw); 
    btQuaternion currentRot = rigidBody->getWorldTransform().getRotation(); 
    btQuaternion deltaRot = targetRotation * currentRot.inverse(); 
    float angle = deltaRot.getAngle(); 
    btVector3 axis = deltaRot.getAxis(); 
    if (angle > SIMD_PI) angle -= SIMD_2_PI; 
    rigidBody->setAngularVelocity(axis * (angle * turnSpeed)); 
} 
void Character::syncRenderTransform() 
{ 
    btTransform t = rigidBody->getWorldTransform(); 
    btVector3 p = t.getOrigin(); 
    btQuaternion r = t.getRotation(); 
    glm::vec3 position(p.x(), p.y(), p.z()); 
    glm::quat rotation(r.w(), r.x(), r.y(), r.z()); 
    float angle = 2.0f * glm::acos(glm::clamp(rotation.w, -1.0f, 1.0f)); 
    glm::vec3 axis(0.0f, 1.0f, 0.0f); 
    if (glm::sin(angle / 2.0f) > 0.001f) 
        axis = glm::normalize(glm::vec3(rotation.x, rotation.y, rotation.z) / glm::sin(angle / 2.0f)); 
    glm::vec3 scale = modelLoader->getModelScale(modelIndex); 
    modelLoader->setModelTransform(modelIndex, position, scale, angle, axis); 
} 
void Character::updateAnimationBlend() 
{ 
    if (!modelLoader) return; 
    if (!blendInitialized) 
    { 
        modelLoader->blendModelAnimations(modelIndex, { 
            {runAnimIndex, 1.0f}, 
            {idleAnimIndex, 0.0f} 
        }); 
        blendInitialized = true; 
    } 
    btVector3 velocity = rigidBody->getLinearVelocity(); 
    float speed = glm::length(glm::vec2(velocity.x(), velocity.z())); 
    const float minSpeed = 0.1f; 
    const float maxSpeed = 5.0f; 
    float runWeight = glm::clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0f, 1.0f); 
    modelLoader->setBlendWeights(modelIndex, {runWeight, 1.0f - runWeight}); 
} 

void Character::takeDamage(float amount)
{
    if (isDead()) return;
    health = glm::max(health - amount, 0.0f);
}

void Character::heal(float amount)
{
    health = glm::min(health + amount, maxHealth);
}
void Character::respawn(const glm::vec3& spawnPos)
{
    health = maxHealth;

    if (!rigidBody) return;


    btTransform t = rigidBody->getWorldTransform();
    t.setOrigin(btVector3(spawnPos.x, spawnPos.y, spawnPos.z));
    rigidBody->setWorldTransform(t);
    rigidBody->setLinearVelocity(btVector3(0, 0, 0));
    rigidBody->setAngularVelocity(btVector3(0, 0, 0));
    rigidBody->activate(true);


    rigidBody->forceActivationState(ACTIVE_TAG);
}