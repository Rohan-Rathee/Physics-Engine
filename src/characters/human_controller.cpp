#include "human_controller.h" 
#include "../systems/input_system.h" 
#include "../camera.h"     
#include <glm/glm.hpp> 
HumanController::HumanController(InputSystem* inputSystem, Camera* camera) 
    : inputSystem(inputSystem), camera(camera) 
{ 
} 
ControlInput HumanController::getInput(float deltaTime, const glm::vec3& /*currentPosition*/) 
{ 
    ControlInput out; 
    if (!inputSystem || !camera) 
        return out; 
    InputState state = inputSystem->getInputState(); 
      
      
    glm::vec3 cameraForward = camera->Front; 
    cameraForward.y = 0.0f; 
    if (glm::length(cameraForward) > 0.0001f) 
        cameraForward = glm::normalize(cameraForward); 
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraForward, camera->WorldUp)); 
    glm::vec3 moveDir = 
        cameraForward * state.moveAxis.y + 
        cameraRight   * state.moveAxis.x; 
    if (glm::length(moveDir) > 0.0001f) 
        moveDir = glm::normalize(moveDir); 
    out.moveDir = moveDir; 
    out.aimDir = camera->Front;         
    out.jumpPressed = state.jumpPressed; 
    out.firePressed = state.firePressed; 
    return out; 
} 
