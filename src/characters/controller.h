#pragma once 
#include <glm/glm.hpp> 
  
  
struct ControlInput 
{ 
    glm::vec3 moveDir{0.0f};                    
    glm::vec3 aimDir{0.0f, 0.0f, -1.0f};         
    bool jumpPressed = false; 
    bool firePressed = false; 
}; 
class IController 
{ 
public: 
    virtual ~IController() = default; 
      
      
    virtual ControlInput getInput(float deltaTime, const glm::vec3& currentPosition) = 0; 
}; 
