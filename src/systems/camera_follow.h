/**
 * @file camera_follow.h
 * @brief Manages the camera following logic for a third-person view.
 *
 * is simple and plan to keep is simple
 */
#pragma once 
class Camera; 
class Character; 
  
  
  
class ThirdPersonCameraRig 
{ 
public: 
    void update(Camera& camera, const Character& target, float deltaTime); 
    float followDistance = 1.0f; 
    float followHeight = -1.0f; 
    float smoothness = 1.0f;  
}; 
