#pragma once
#include <glm/glm.hpp>

class Engine;
class Character;


class IGame
{
public:
    virtual ~IGame() = default;


    virtual bool onInitialize(Engine& engine) = 0;

    virtual void onUpdate(float deltaTime) = 0;

    virtual void onRenderUI() {}

    virtual Character* getCameraTarget() { return nullptr; }

    virtual void onShutdown() {}
};