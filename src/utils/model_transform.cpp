#include "model_transform.h"
#include "model_loader.h"

#include "../systems/physics_system.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>

ModelTransform::ModelTransform(ModelLoader *modelLoader, PhysicsSystem *physicsSystem)
    : modelLoader(modelLoader), physicsSystem(physicsSystem) {
    if (!modelLoader) {
        std::cerr << "ModelTransform: ModelLoader pointer is null!" << std::endl;
    }

    if (!physicsSystem) {
        std::cerr << "ModelTransform: PhysicsSystem pointer is null!" << std::endl;
    }
}

ModelTransform::~ModelTransform() {
    for (const auto &physicsModel : physicsModels) {
        if (physicsSystem && physicsModel.rigidBody) {
            physicsSystem->removeRigidBody(physicsModel.rigidBody);
        }
    }
}

void ModelTransform::clear() {
    for (const auto &physicsModel : physicsModels) {
        if (physicsSystem && physicsModel.rigidBody) {
            physicsSystem->removeRigidBody(physicsModel.rigidBody);
        }
    }
    physicsModels.clear();
}

void ModelTransform::setTransform(size_t modelIndex, const glm::vec3 &position, const glm::vec3 &scale, float rotationAngle, const glm::vec3 &rotationAxis) {
    if (!modelLoader) {
        std::cerr << "ModelTransform::setTransform: ModelLoader is not initialized!" << std::endl;

        return;
    }
    modelLoader->setModelTransform(modelIndex, position, scale, rotationAngle, rotationAxis);
}

void ModelTransform::updateTransform(size_t modelIndex, const glm::vec3 &deltaPosition, const glm::vec3 &deltaScale, float deltaRotationAngle, const glm::vec3 &rotationAxis) {
    if (!modelLoader) {
        std::cerr << "ModelTransform::updateTransform: ModelLoader is not initialized!" << std::endl;

        return;
    }
    modelLoader->updateModelTransform(modelIndex, deltaPosition, deltaScale, deltaRotationAngle, rotationAxis);
}

void ModelTransform::initializePhysicsBody(size_t modelIndex, float mass, btCollisionShape *shape, float restitution) {
    if (!physicsSystem) {
        std::cerr << "ModelTransform::initializePhysicsBody: PhysicsSystem is not initialized!" << std::endl;
        return;
    }

    if (!modelLoader) {
        std::cerr << "ModelTransform::initializePhysicsBody: ModelLoader is not initialized!" << std::endl;
        return;
    }

    glm::vec3 position = modelLoader->getModelPosition(modelIndex);
    btRigidBody *rigidBody = physicsSystem->createRigidBody(mass, shape, position, restitution);
    if (rigidBody) {

        PhysicsModelData data;
        data.rigidBody = rigidBody;
        data.collisionShape = shape;
        data.modelIndex = modelIndex;

        physicsModels.push_back(data);
    } else {
        std::cerr << "Failed to create rigid body for model " << modelIndex << std::endl;
    }
}

void ModelTransform::applyPhysicsTransform(const PhysicsModelData &physicsModel) {

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

    if (glm::sin(angle / 2.0f) > 0.001f) {
        axis.x = rotation.x / glm::sin(angle / 2.0f);
        axis.y = rotation.y / glm::sin(angle / 2.0f);
        axis.z = rotation.z / glm::sin(angle / 2.0f);
        axis = glm::normalize(axis);
    }

    glm::vec3 scale = modelLoader->getModelScale(physicsModel.modelIndex);
    setTransform(physicsModel.modelIndex, position, scale, angle, axis);
}

btRigidBody *ModelTransform::getPhysicsBody(size_t modelIndex) {
    for (const auto &physicsModel : physicsModels) {

        if (physicsModel.modelIndex == modelIndex) {
            return physicsModel.rigidBody;
        }
    }

    return nullptr;
}

void ModelTransform::applyImpulse(size_t modelIndex, const glm::vec3 &impulse) {
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

void ModelTransform::applyForce(size_t modelIndex, const glm::vec3 &force) {

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

void ModelTransform::applyTorque(size_t modelIndex, const glm::vec3 &torque) {

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