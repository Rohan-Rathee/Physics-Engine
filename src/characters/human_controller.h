/**
 * @file human_controller.h
 * @brief Defines the HumanController class for handling player input. is actually suprisingly simple, just gets input from the input system and returns it as a ControlInput struct. atleast more simple than the bot and ai stuff.
 * 
 */


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