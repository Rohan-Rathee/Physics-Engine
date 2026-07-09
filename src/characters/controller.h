/**
 * @file controller.h
 * @brief a helper/guide class that provides a common interface for different types of controllers (human, bot, ai) and allows bot input and human input to be handled in a similar way from here on.
 * //newer class (relatively) comments are getting more and more no shit sherlock type, maybe because the newer files are more self explanatory and self contained with one job unlike the older files that are a lot
 */
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
