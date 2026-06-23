#pragma once 
#include "controller.h" 
class InputSystem; 
class Camera; 
  
  
  
class HumanController : public IController 
{ 
public: 
    HumanController(InputSystem* inputSystem, Camera* camera); 
    ControlInput getInput(float deltaTime, const glm::vec3& currentPosition) override; 
private: 
    InputSystem* inputSystem; 
    Camera* camera; 
}; 
