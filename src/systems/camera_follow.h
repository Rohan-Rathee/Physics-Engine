#pragma once 
class Camera; 
class Character; 
  
  
  
class ThirdPersonCameraRig 
{ 
public: 
    void update(Camera& camera, const Character& target, float deltaTime); 
    float followDistance = 2.0f; 
    float followHeight = -0.5f; 
    float smoothness = 1.0f;  
}; 
