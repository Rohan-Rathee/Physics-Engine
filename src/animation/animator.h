#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "animation.h"

class Animator {
public:
    Animator();


    void PlayAnimation(Animation* animation);


    void UpdateAnimation(float deltaTime);

    const std::vector<glm::mat4>& GetFinalBoneMatrices() const
    {
        return finalBoneMatrices;
    }

    Animation* GetCurrentAnimation() const { return currentAnimation; }

private:


    void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform);

    std::vector<glm::mat4> finalBoneMatrices;
    Animation* currentAnimation = nullptr;
    float currentTime = 0.0f;
};

#endif
