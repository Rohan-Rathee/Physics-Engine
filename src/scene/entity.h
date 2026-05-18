#ifndef ENTITY_H
#define ENTITY_H

#include "../components/transform.h"
#include <memory>
#include <string>

class Entity {
protected:
    std::string name;
    Transform transform;
    bool active;

public:
    Entity(const std::string& entityName = "Entity");
    virtual ~Entity() = default;
    
    Transform& getMatrix() { return transform; }
    const Transform& getTransform() const { return transform; }
    
    void setActive(bool a) { active = a; }
    bool isActive() const { return active; }
    
    std::string getName() const { return name; }
    void setName(const std::string& n) { name = n; }    
    
    virtual void update(float deltaTime) {}
    virtual void render() {}
};

#endif
