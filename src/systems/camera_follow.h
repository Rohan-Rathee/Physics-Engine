#pragma once 
class Camera; 
class Character; 
  
  
  
class ThirdPersonCameraRig 
{ 
public: 
    void update(Camera& camera, const Character& target, float deltaTime); 
    float followDistance = 0.1f; 
    float followHeight = -0.8f; 
    float smoothness = 1.0f;  
}; 
