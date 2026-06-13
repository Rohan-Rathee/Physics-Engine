#include "model_transform.h"
#include "model_loader.h"
#include "../systems/physics_system.h"
#include "../camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <glfw/glfw3.h>

// Forward declare global camera (link from main.cpp)
extern Camera *g_camera;

ModelTransform::ModelTransform(ModelLoader *modelLoader, PhysicsSystem *physicsSystem)
    : modelLoader(modelLoader), physicsSystem(physicsSystem)
{
    if (!modelLoader)
    {
        std::cerr << "ModelTransform: ModelLoader pointer is null!" << std::endl;
    }
    if (!physicsSystem)
    {
        std::cerr << "ModelTransform: PhysicsSystem pointer is null!" << std::endl;
    }
}

ModelTransform::~ModelTransform()
{
    // Cleanup physics bodies
    for (const auto &physicsModel : physicsModels)
    {
        if (physicsSystem && physicsModel.rigidBody)
        {
            physicsSystem->removeRigidBody(physicsModel.rigidBody);
        }
    }
}

void ModelTransform::setTransform(size_t modelIndex, const glm::vec3 &position, const glm::vec3 &scale,
                                  float rotationAngle, const glm::vec3 &rotationAxis)
{
    if (!modelLoader)
    {
        std::cerr << "ModelTransform::setTransform: ModelLoader is not initialized!" << std::endl;
        return;
    }

    modelLoader->setModelTransform(modelIndex, position, scale, rotationAngle, rotationAxis);
}

void ModelTransform::updateTransform(size_t modelIndex, const glm::vec3 &deltaPosition, const glm::vec3 &deltaScale,
                                     float deltaRotationAngle, const glm::vec3 &rotationAxis)
{
    if (!modelLoader)
    {
        std::cerr << "ModelTransform::updateTransform: ModelLoader is not initialized!" << std::endl;
        return;
    }

    modelLoader->updateModelTransform(modelIndex, deltaPosition, deltaScale, deltaRotationAngle, rotationAxis);
}

void ModelTransform::initializePhysicsBody(size_t modelIndex, float mass, btCollisionShape *shape, float restitution)
{
    if (!physicsSystem)
    {
        std::cerr << "ModelTransform::initializePhysicsBody: PhysicsSystem is not initialized!" << std::endl;
        return;
    }

    if (!modelLoader)
    {
        std::cerr << "ModelTransform::initializePhysicsBody: ModelLoader is not initialized!" << std::endl;
        return;
    }

    // Get position from modelLoader's stored transform
    glm::vec3 position = modelLoader->getModelPosition(modelIndex);

    // Create rigid body in physics system at the stored position
    btRigidBody *rigidBody = physicsSystem->createRigidBody(mass, shape, position, restitution);

    if (rigidBody)
    {
        PhysicsModelData data;
        data.rigidBody = rigidBody;
        data.collisionShape = shape;
        data.modelIndex = modelIndex;
        physicsModels.push_back(data);
    }
    else
    {
        std::cerr << "Failed to create rigid body for model " << modelIndex << std::endl;
    }
}

void ModelTransform::applyPhysicsTransform(const PhysicsModelData &physicsModel)
{
    if (!physicsModel.rigidBody)
        return;

    // Get transform from rigid body
    btTransform btTrans;
    physicsModel.rigidBody->getMotionState()->getWorldTransform(btTrans);

    // Extract position
    btVector3 btPos = btTrans.getOrigin();
    glm::vec3 position(btPos.x(), btPos.y(), btPos.z());

    // Extract rotation as quaternion
    btQuaternion btRot = btTrans.getRotation();
    glm::quat rotation(btRot.w(), btRot.x(), btRot.y(), btRot.z());

    float angle = 2.0f * glm::acos(glm::clamp(rotation.w, -1.0f, 1.0f));
    glm::vec3 axis(0.0f, 1.0f, 0.0f);

    if (glm::sin(angle / 2.0f) > 0.001f)
    {
        axis.x = rotation.x / glm::sin(angle / 2.0f);
        axis.y = rotation.y / glm::sin(angle / 2.0f);
        axis.z = rotation.z / glm::sin(angle / 2.0f);
        axis = glm::normalize(axis);
    }

    glm::vec3 scale = glm::vec3(1.0f); // For now, we don't extract scale from physics, but you could if needed
    setTransform(
        physicsModel.modelIndex,
        position,
        scale,
        angle,
        axis);
}

void ModelTransform::updateFrameTransforms(float deltaTime)
{
    if (!modelLoader)
    {
        std::cerr << "ModelTransform::updateFrameTransforms: ModelLoader is not initialized!" << std::endl;
        return;
    }

    if (!physicsSystem)
    {
        std::cerr << "ModelTransform::updateFrameTransforms: PhysicsSystem is not initialized!" << std::endl;
        return;
    }

    if (physicsModels.size() > 1)
    {
        btRigidBody *modelBody = physicsModels[1].rigidBody;
        if (modelBody)
        {
            // Get model's rotation to calculate its local forward direction
            btTransform btTrans;
            modelBody->getMotionState()->getWorldTransform(btTrans);
            btQuaternion btRot = btTrans.getRotation();
            glm::quat modelRotation(btRot.w(), btRot.x(), btRot.y(), btRot.z());

            glm::vec3 cameraForward = g_camera->Front;
            cameraForward.y = 0.0f;
            cameraForward = glm::normalize(cameraForward);
            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraForward, g_camera->WorldUp));
            glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraForward));

            glm::vec3 modelFront = glm::normalize(modelRotation * glm::vec3(-1.0f, 0.0f, 0.0f));
            glm::vec3 modelUp = glm::normalize(modelRotation * glm::vec3(0.0f, -1.0f, 0.0f));
            int keypress = 0;
            // UP/DOWN: apply force in model's forward/backward direction
            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS)
            {
                applyForce(physicsModels[1].modelIndex, cameraForward * 800.0f);
                keypress = 1;
            }
            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS)
            {
                applyForce(physicsModels[1].modelIndex, -cameraForward * 400.0f);
                keypress = 1;
            }
            // LEFT/RIGHT: apply torque around model's up direction
            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS)
            {
                applyForce(physicsModels[1].modelIndex, -cameraRight * 400.0f);
                keypress = 1;
            }
            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS)
            {
                applyForce(physicsModels[1].modelIndex, cameraRight * 400.0f);
                keypress = 1;
            }
            btVector3 startPos = modelBody->getWorldTransform().getOrigin();
            btVector3 endPos = startPos - btVector3(0, 1.2f, 0); // Ray length = slightly more than half-height

            btCollisionWorld::ClosestRayResultCallback rayCallback(startPos, endPos);
            physicsSystem->getDynamicsWorld()->rayTest(startPos, endPos, rayCallback);

            if (rayCallback.hasHit())
            {
                isModelInAir = false;
            }
            else
            {
                isModelInAir = true;
            }
            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS and !isModelInAir)
            {
                applyImpulse(physicsModels[1].modelIndex, cameraUp * 200.0f);
                isModelInAir = true; // Set in-air flag when jump impulse is applied
                keypress = 1;
            }

            // turn character to face camera direction always (optional)
            // =========// 1. Calculate Yaw (Turning)
            // 1. Calculate Yaw (Turning)
            glm::vec3 desiredForward = cameraForward;
            desiredForward.y = 0.0f;
            btQuaternion finalRotation = btQuaternion::getIdentity();

            if (glm::length(desiredForward) > 0.001f)
            {
                desiredForward = glm::normalize(desiredForward);
                float yaw = -atan2(desiredForward.z, desiredForward.x) + glm::radians(180.0f);

                // We only use the Yaw rotation now
                finalRotation = btQuaternion(btVector3(0.0f, 1.0f, 0.0f), yaw);
            }

            // 2. Apply using Physics (Angular Velocity)
            btTransform currentTransform;
            physicsModels[1].rigidBody->getMotionState()->getWorldTransform(currentTransform);
            btQuaternion currentRot = currentTransform.getRotation();

            // Calculate angular velocity required to reach the target Yaw
            btQuaternion deltaRot = finalRotation * currentRot.inverse();
            float angle = deltaRot.getAngle();
            btVector3 axis = deltaRot.getAxis();

            if (angle > SIMD_PI)
                angle -= SIMD_2_PI;

            // 'turnSpeed' controls how fast the bike snaps to the camera direction
            float turnSpeed = 5.0f;
            physicsModels[1].rigidBody->setAngularVelocity(axis * (angle * turnSpeed));

            physicsSystem->update(deltaTime);
            for (const auto &physicsModel : physicsModels)
            {
                applyPhysicsTransform(physicsModel);
            }

            if (physicsModels.size() > targetModelIndex)
            {
                const auto &targetPhysics = physicsModels[targetModelIndex];
                if (targetPhysics.rigidBody && g_camera)
                {

                    btTransform btTrans;
                    targetPhysics.rigidBody->getMotionState()->getWorldTransform(btTrans);

                    btVector3 btPos = btTrans.getOrigin();
                    btQuaternion btRot = btTrans.getRotation();

                    glm::vec3 modelPos(btPos.x(), btPos.y(), btPos.z());
                    glm::vec3 p = modelPos;
                    glm::quat modelRot(btRot.w(), btRot.x(), btRot.y(), btRot.z());

                    // === YOUR BIKE'S AXES ===
                    glm::vec3 vehicleForward = glm::normalize(modelRot * glm::vec3(-1.0f, 0.0f, 0.0f)); // X = Forward
                    glm::vec3 vehicleUp = glm::normalize(modelRot * glm::vec3(0.0f, -1.0f, 0.0f));      // Y = Up

                    // Camera position behind the bike
                    float yawRad = glm::radians(g_camera->Yaw);
                    float pitchRad = glm::radians(g_camera->Pitch);

                    glm::vec3 orbitDir;

                    orbitDir.x = cos(yawRad) * cos(pitchRad); // Horizontal orbit direction
                    orbitDir.y = sin(pitchRad);               // Vertical orbit direction
                    orbitDir.z = sin(yawRad) * cos(pitchRad); // Horizontal orbit direction

                    orbitDir = glm::normalize(orbitDir);

                    glm::vec3 targetPoint =
                        modelPos +
                        vehicleUp * followHeight;

                    // Orbit around bike
                    glm::vec3 desiredPos =
                        targetPoint -
                        orbitDir * followDistance;

                    // Desired look direction (slightly ahead of bike)
                    glm::vec3 desiredFront =
                        glm::normalize(targetPoint - desiredPos);

                    // === SMOOTH POSITION ===
                    g_camera->Position = glm::mix(g_camera->Position, desiredPos, smoothness);

                    // === FIX ROTATION PROPERLY ===
                    g_camera->Front = glm::mix(g_camera->Front, desiredFront, smoothness * 1.8f); // Faster rotation
                    g_camera->Front = glm::normalize(g_camera->Front);

                    // Rebuild Right and Up vectors
                    g_camera->Right = glm::normalize(glm::cross(g_camera->Front, g_camera->WorldUp));
                    g_camera->Up = glm::normalize(glm::cross(g_camera->Right, g_camera->Front));

                    // Optional: Update Yaw & Pitch so mouse control doesn't fight
                    // g_camera->Yaw   = glm::degrees(atan2(g_camera->Front.z, g_camera->Front.x));
                    g_camera->Pitch = glm::degrees(asin(g_camera->Front.y));
                }
            }
        }
    }
}

        btRigidBody *ModelTransform::getPhysicsBody(size_t modelIndex)
        {
            for (const auto &physicsModel : physicsModels)
            {
                if (physicsModel.modelIndex == modelIndex)
                {
                    return physicsModel.rigidBody;
                }
            }
            return nullptr;
        }

        void ModelTransform::applyImpulse(size_t modelIndex, const glm::vec3 &impulse)
        {
            btRigidBody *body = getPhysicsBody(modelIndex);

            if (!body)
                return;

            body->activate(true);

            body->applyCentralImpulse(
                btVector3(
                    impulse.x,
                    impulse.y,
                    impulse.z));
        }
        void ModelTransform::applyForce(size_t modelIndex, const glm::vec3 &force)
        {
            btRigidBody *body = getPhysicsBody(modelIndex);

            if (!body)
                return;

            body->activate(true);

            body->applyCentralForce(
                btVector3(
                    force.x,
                    force.y,
                    force.z));
        }
        void ModelTransform::applyTorque(size_t modelIndex, const glm::vec3 &torque)
        {
            btRigidBody *body = getPhysicsBody(modelIndex);

            if (!body)
                return;

            body->activate(true);

            body->applyTorque(
                btVector3(
                    torque.x,
                    torque.y,
                    torque.z));
        }
