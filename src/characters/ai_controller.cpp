#include "ai_controller.h" 
AIController::AIController(std::vector<glm::vec3> patrolPoints) 
    : patrolPoints(std::move(patrolPoints)) 
{ 
} 
ControlInput AIController::getInput(float deltaTime, const glm::vec3& currentPosition) 
{ 
    if (target) 
    { 
        float dist = glm::length(*target - currentPosition); 
        if (dist <= chaseRange) 
            return chase(currentPosition, *target); 
    } 
    return patrol(currentPosition); 
} 
ControlInput AIController::patrol(const glm::vec3& currentPosition) 
{ 
    ControlInput out; 
    if (patrolPoints.empty()) 
        return out;   
    glm::vec3 toWaypoint = patrolPoints[patrolIndex] - currentPosition; 
    toWaypoint.y = 0.0f; 
    if (glm::length(toWaypoint) <= waypointRadius) 
    { 
        patrolIndex = (patrolIndex + 1) % patrolPoints.size(); 
        return out;   
    } 
    out.moveDir = glm::normalize(toWaypoint); 
    out.aimDir = out.moveDir; 
    return out; 
} 
ControlInput AIController::chase(const glm::vec3& currentPosition, const glm::vec3& targetPos) 
{ 
    ControlInput out; 
    glm::vec3 toTarget = targetPos - currentPosition; 
    toTarget.y = 0.0f; 
    if (glm::length(toTarget) > 0.001f) 
        out.moveDir = glm::normalize(toTarget); 
    out.aimDir = out.moveDir; 
      
    return out; 
} 
