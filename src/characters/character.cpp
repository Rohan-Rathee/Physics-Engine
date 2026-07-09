#include "character.h" 
#include "../utils/model_loader.h" 
#include "../systems/physics_system.h"     
#include <btBulletDynamicsCommon.h> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/quaternion.hpp> 
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/quaternion.hpp> 

namespace
{
    // Hitscan weapon tuning — adjust to taste.
    constexpr float kWeaponRange   = 200.0f; // max ray distance (world units)
    constexpr float kWeaponDamage  = 20.0f;  // damage applied per hit
    constexpr float kFireRate      = 0.15f;  // seconds between shots
    constexpr float kEyeHeight     = 0.9f;   // ray origin height above feet
    constexpr float kMuzzleOffset  = 0.6f;   // push origin forward so we don't hit our own collider
    constexpr float kTracerLifetime = 0.05f; // how long m_lastShot* stays valid, for optional tracer rendering
}

Character::Character(size_t modelIndex, 
                      btRigidBody* rigidBody, 
                      ModelLoader* modelLoader, 
                      PhysicsSystem* physicsSystem, 
                      std::unique_ptr<IController> controller, 
                      unsigned int runAnimIndex, 
                      unsigned int idleAnimIndex,
                      unsigned int shootAnimIndex,
                      bool hasShootAnim) 
    : modelIndex(modelIndex), 
      rigidBody(rigidBody), 
      modelLoader(modelLoader), 
      physicsSystem(physicsSystem), 
      controller(std::move(controller)), 
      runAnimIndex(runAnimIndex), 
      idleAnimIndex(idleAnimIndex),
      shootAnimIndex(shootAnimIndex),
      hasShootAnim(hasShootAnim) 

{ 
    if (rigidBody)
        rigidBody->setUserPointer(static_cast<void*>(this));

    if (modelLoader)
        baseScale = modelLoader->getModelScale(modelIndex); // cache before we ever touch it
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

    if (hasShootAnim){
        if (input.firePressed) {
            shootTimer = shootAnimDuration;
        }
        else
            shootTimer = glm::max(shootTimer - deltaTime, 0.0f);
    }
        handleShooting(input, deltaTime);
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

    glm::vec3 scale = isDead() ? glm::vec3(0.0f) : baseScale;
    modelLoader->setModelTransform(modelIndex, position, scale, angle, axis); 
}
void Character::updateAnimationBlend() 
{ 
    if (!modelLoader) return; 

    if (!blendInitialized) 
    { 
        if (hasShootAnim)
        {
            modelLoader->blendModelAnimations(modelIndex, { 
                {runAnimIndex, 1.0f}, 
                {idleAnimIndex, 0.0f},
                {shootAnimIndex, 0.0f}
            }); 
        }
        else
        {
            modelLoader->blendModelAnimations(modelIndex, { 
                {runAnimIndex, 1.0f}, 
                {idleAnimIndex, 0.0f} 
            }); 
        }
        blendInitialized = true; 
    } 

    btVector3 velocity = rigidBody->getLinearVelocity(); 
    float speed = glm::length(glm::vec2(velocity.x(), velocity.z())); 
    const float minSpeed = 0.1f; 
    const float maxSpeed = 5.0f; 
    float runWeight = glm::clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0f, 1.0f); 

    if (hasShootAnim)
    {

        float shootWeight = glm::clamp(shootTimer / shootAnimDuration, 0.0f, 1.0f);
        float moveWeight = 1.0f - shootWeight;
        modelLoader->setBlendWeights(modelIndex, {
            runWeight * moveWeight,
            (1.0f - runWeight) * moveWeight,
            shootWeight
        });
    }
    else
    {
        modelLoader->setBlendWeights(modelIndex, {runWeight, 1.0f - runWeight}); 
    }
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


void Character::handleShooting(const ControlInput& input, float deltaTime)
{

    if (m_fireCooldown > 0.0f)
        m_fireCooldown -= deltaTime;
 
    if (!input.firePressed || m_fireCooldown > 0.0f)
        return;
    if (!rigidBody || !physicsSystem || isDead())
        return;
 
    m_fireCooldown = kFireRate;
    shootTimer = shootAnimDuration;

 
    glm::vec3 dir = input.aimDir;

    if (glm::length(dir) < 0.0001f)
        dir = getForward();
    dir = glm::normalize(dir);

    glm::vec3 origin = getPosition() + glm::vec3(0.0f, kEyeHeight, 0.0f) + dir * kMuzzleOffset;
    glm::vec3 end     = origin + dir * kWeaponRange;
 
    btVector3 btFrom(origin.x, origin.y, origin.z);
    btVector3 btTo(end.x, end.y, end.z);
 
    btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
    physicsSystem->getDynamicsWorld()->rayTest(btFrom, btTo, rayCallback);
 
    m_lastShotOrigin = origin;
    m_lastShotEnd    = rayCallback.hasHit()
        ? glm::vec3(rayCallback.m_hitPointWorld.x(),
                    rayCallback.m_hitPointWorld.y(),
                    rayCallback.m_hitPointWorld.z())
        : end;
    m_lastShotTimer = kTracerLifetime;
 
    if (!rayCallback.hasHit())
        return;
 
    const btCollisionObject* hitObject = rayCallback.m_collisionObject;
    if (hitObject == rigidBody)
        return; // safety guard, shouldn't happen given the muzzle offset
 
    Character* victim = static_cast<Character*>(hitObject->getUserPointer());
    if (!victim || victim == this || victim->isDead())
        return;
 
    victim->takeDamage(kWeaponDamage);
}