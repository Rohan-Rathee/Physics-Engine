#include "model_transform.h"
#include "model_loader.h"
#include "../systems/physics_system.h"
#include "../camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <glfw/glfw3.h>


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


    glm::vec3 position = modelLoader->getModelPosition(modelIndex);


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


    btTransform btTrans;
    physicsModel.rigidBody->getMotionState()->getWorldTransform(btTrans);


    btVector3 btPos = btTrans.getOrigin();
    glm::vec3 position(btPos.x(), btPos.y(), btPos.z());


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

    glm::vec3 scale = modelLoader->getModelScale(physicsModel.modelIndex);
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
            bool forwardInput = 0;
            bool lateralInput = 0;
            glm::vec3 moveDir(0.0f);

            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS)
            {
                moveDir += cameraForward;
                forwardInput = true;
            }

            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS)
            {
                moveDir -= cameraForward;
                forwardInput = true;
            }

            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_A) == GLFW_PRESS)
            {
                moveDir -= cameraRight;
                lateralInput = true;
            }

            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_D) == GLFW_PRESS)
            {
                moveDir += cameraRight;
                lateralInput = true;
            }

            if (glm::length(moveDir) > 0.001f)
            {
                moveDir = glm::normalize(moveDir);
            }

            btVector3 startPos = modelBody->getWorldTransform().getOrigin();
            btVector3 endPos = startPos - btVector3(0, 1.0f, 0);

            btCollisionWorld::ClosestRayResultCallback rayCallback(startPos, endPos);
            physicsSystem->getDynamicsWorld()->rayTest(startPos, endPos, rayCallback);


            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS and !isModelInAir)
            {
                applyImpulse(physicsModels[1].modelIndex, cameraUp * 500.0f);
                isModelInAir = true;
                keypress = 1;
            }

            if (rayCallback.hasHit())  
            {
                isModelInAir = false;
            }
            else
            {
                isModelInAir = true;
            }

            if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_SPACE) == GLFW_PRESS)
            {
                isModelInAir = true;
            }

            btVector3 vel = modelBody->getLinearVelocity();

            const float moveSpeed = 8.0f;

            if (!isModelInAir)
            {
                glm::vec2 currentVel(
                    vel.x(),
                    vel.z());

                glm::vec2 targetVel(0.0f);

                if (glm::length(moveDir) > 0.001f)
                {
                    targetVel =
                        glm::vec2(
                            moveDir.x * moveSpeed,
                            moveDir.z * moveSpeed);
                }

                float acceleration = 12.0f;
                float deceleration = 4.0f;

                float blend =
                    glm::length(targetVel) > glm::length(currentVel)
                        ? acceleration
                        : deceleration;

                currentVel = glm::mix(
                    currentVel,
                    targetVel,
                    blend * deltaTime);

                modelBody->setLinearVelocity(
                    btVector3(
                        currentVel.x,
                        vel.y(),
                        currentVel.y));
            }
            else
            {

                if (glm::length(moveDir) > 0.001f)
                {
                    applyForce(
                        physicsModels[1].modelIndex,
                        moveDir * 200.0f);
                }
            }



            glm::vec3 desiredForward = cameraForward;
            desiredForward.y = 0.0f;
            btQuaternion finalRotation = btQuaternion::getIdentity();

            if (glm::length(desiredForward) > 0.001f)
            {
                desiredForward = glm::normalize(desiredForward);
                float yaw = -atan2(desiredForward.z, desiredForward.x) + glm::radians(180.0f);


                finalRotation = btQuaternion(btVector3(0.0f, 1.0f, 0.0f), yaw);
            }


            btTransform currentTransform;
            physicsModels[1].rigidBody->getMotionState()->getWorldTransform(currentTransform);
            btQuaternion currentRot = currentTransform.getRotation();


            btQuaternion deltaRot = finalRotation * currentRot.inverse();
            float angle = deltaRot.getAngle();
            btVector3 axis = deltaRot.getAxis();

            if (angle > SIMD_PI)
                angle -= SIMD_2_PI;


            float turnSpeed = 50.0f;
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


                    glm::vec3 vehicleForward = glm::normalize(modelRot * glm::vec3(-1.0f, 0.0f, 0.0f));
                    glm::vec3 vehicleUp = glm::normalize(modelRot * glm::vec3(0.0f, -1.0f, 0.0f));


                    float yawRad = glm::radians(g_camera->Yaw);
                    float pitchRad = glm::radians(g_camera->Pitch);

                    glm::vec3 orbitDir;

                    orbitDir.x = cos(yawRad) * cos(pitchRad);
                    orbitDir.y = sin(pitchRad);
                    orbitDir.z = sin(yawRad) * cos(pitchRad);

                    orbitDir = glm::normalize(orbitDir);

                    glm::vec3 targetPoint =
                        modelPos +
                        vehicleUp * followHeight;


                    glm::vec3 desiredPos =
                        targetPoint -
                        orbitDir * followDistance;


                    glm::vec3 desiredFront =
                        glm::normalize(targetPoint - desiredPos);


                    g_camera->Position = glm::mix(g_camera->Position, desiredPos, smoothness);


                    g_camera->Front = glm::mix(g_camera->Front, desiredFront, smoothness * 1.8f);
                    g_camera->Front = glm::normalize(g_camera->Front);


                    g_camera->Right = glm::normalize(glm::cross(g_camera->Front, g_camera->WorldUp));
                    g_camera->Up = glm::normalize(glm::cross(g_camera->Right, g_camera->Front));



                    g_camera->Pitch = glm::degrees(asin(g_camera->Front.y));
                }
            }
        }

        if (physicsModels.size() > 1)
        {
            btRigidBody *modelBody = physicsModels[1].rigidBody;
            if (modelBody)
            {

                if (!blendAnimationsInitialized)
                {
                    modelLoader->blendModelAnimations(1, {
                                                             {0u, 1.0f},
                                                             {1u, 0.0f}
                                                         });
                    blendAnimationsInitialized = true;
                }

                btVector3 velocity = modelBody->getLinearVelocity();
                float speed =
                    glm::length(
                        glm::vec2(
                            velocity.x(),
                            velocity.z()));

                const float minSpeed = 0.1f;
                const float maxSpeed = 5.0f;

                float runWeight = glm::clamp(
                    (speed - minSpeed) / (maxSpeed - minSpeed),
                    0.0f, 1.0f);


                modelLoader->setBlendWeights(1, {runWeight, 1.0f - runWeight});
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
