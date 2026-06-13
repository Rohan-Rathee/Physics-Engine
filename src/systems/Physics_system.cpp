#include "physics_system.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>

PhysicsSystem::PhysicsSystem() = default;

PhysicsSystem::~PhysicsSystem() {
    shutdown();
}

bool PhysicsSystem::initialize(const glm::vec3& gravityVec) {
    // Initialize Bullet physics components
    collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
    broadphase = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get()
    );
    
    setGravity(gravityVec);
    
    return true;
}
void PhysicsSystem::update(float deltaTime) {
    accumulator += deltaTime;

    while(accumulator >= fixedStep)
    {
        dynamicsWorld->stepSimulation(fixedStep, 0);
        accumulator -= fixedStep;
    }
}


void PhysicsSystem::shutdown() {
    if (dynamicsWorld) {
        // Remove all bodies
        // We need to remove bodies before deleting the world to avoid memory leaks
        //and ensure we clean up motion states and shapes

        for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState()) {
                delete body->getMotionState();
            }
            dynamicsWorld->removeCollisionObject(obj);
            delete obj;
        }
    }
}

btRigidBody* PhysicsSystem::createRigidBody(float mass, btCollisionShape* shape,
                                             const glm::vec3& position, float restitution) {
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));
    

    btMotionState* motionState = new btDefaultMotionState(transform);
    btVector3 localInertia(0, 0, 0);
    
    if (mass != 0.0f) {
        shape->calculateLocalInertia(mass, localInertia);
    }
    
    // Use the RigidBodyConstructionInfo constructor
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    
    // Set restitution for bouncing
    body->setRestitution(restitution);
    body->setFriction(0.5f); // Optional: set some friction
    body->setRollingFriction(0.01f); // Optional: set some rolling friction
    body->setSpinningFriction(1.0f); // Optional: set some spinning friction
    body->setDamping(0.5f, 0.1f);
    
    dynamicsWorld->addRigidBody(body);
    //body->setAngularVelocity(btVector3(50, 10, 0));// Optional: give it some initial spin for visual interest
    
    return body;
}

void PhysicsSystem::removeRigidBody(btRigidBody* body) {
    if (body) {
        dynamicsWorld->removeRigidBody(body);
        if (body->getMotionState()) {
            delete body->getMotionState();
        }
        delete body;
    }
}

void PhysicsSystem::setGravity(const glm::vec3& g) {
    gravity = g;
    if (dynamicsWorld) {
        dynamicsWorld->setGravity(btVector3(gravity.x, gravity.y, gravity.z));
    }
}

