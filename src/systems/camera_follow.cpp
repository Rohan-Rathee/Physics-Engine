#include "camera_follow.h" 
#include "../camera.h"     
#include "../characters/character.h" 
#include <glm/glm.hpp> 
void ThirdPersonCameraRig::update(Camera& camera, const Character& target, float deltaTime) 
{ 
    glm::vec3 modelPos = target.getPosition(); 
    glm::vec3 vehicleUp = target.getUp(); 
    float yawRad = glm::radians(camera.Yaw); 
    float pitchRad = glm::radians(camera.Pitch); 
    glm::vec3 orbitDir( 
        cos(yawRad) * cos(pitchRad), 
        sin(pitchRad), 
        sin(yawRad) * cos(pitchRad) 
    ); 
    orbitDir = glm::normalize(orbitDir); 
    glm::vec3 targetPoint = modelPos + vehicleUp * followHeight; 
    glm::vec3 desiredPos = targetPoint - orbitDir * followDistance; 
    glm::vec3 desiredFront = glm::normalize(targetPoint - desiredPos); 
    camera.Position = glm::mix(camera.Position, desiredPos, smoothness); 
    camera.Front = glm::normalize(glm::mix(camera.Front, desiredFront, smoothness * 1.8f)); 
    camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp)); 
    camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front)); 
    camera.Pitch = glm::degrees(asin(camera.Front.y)); 
} 
