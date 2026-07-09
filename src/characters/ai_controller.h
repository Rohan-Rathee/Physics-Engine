#pragma once 
#include "controller.h" 
#include <glm/glm.hpp> 
#include <vector> 
  

class AIController : public IController 
{ 
public: 
    explicit AIController(std::vector<glm::vec3> patrolPoints = {}); 
    ControlInput getInput(float deltaTime, const glm::vec3& currentPosition) override; 

    void setTarget(const glm::vec3* targetPosition) { target = targetPosition; } 
private: 
    ControlInput patrol(const glm::vec3& currentPosition); 
    ControlInput chase(const glm::vec3& currentPosition, const glm::vec3& targetPos); 
    std::vector<glm::vec3> patrolPoints; 
    size_t patrolIndex = 0; 
    const glm::vec3* target = nullptr; 
    float waypointRadius = 1.5f; 
    float chaseRange = 25.0f; 
}; 
