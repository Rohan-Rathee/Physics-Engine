#include "scene.h"
#include <algorithm>

Scene::Scene(const std::string& sceneName) : name(sceneName) {}

std::shared_ptr<Entity> Scene::addEntity(std::shared_ptr<Entity> entity) {
    entities.push_back(entity);
    return entity;
}

void Scene::removeEntity(std::shared_ptr<Entity> entity) {
    entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
}

void Scene::update(float deltaTime) {
    for (auto& entity : entities) {
        if (entity->isActive()) {
            entity->update(deltaTime);
        }
    }
}

void Scene::render() {
    for (auto& entity : entities) {
        if (entity->isActive()) {
            entity->render();
        }
    }
}
