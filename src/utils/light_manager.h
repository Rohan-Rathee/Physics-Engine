#pragma once




















#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <string>
#include <vector>
#include <array>
#include <glm/glm.hpp>


class Shader;




static constexpr int MAX_LIGHTS = 16;




enum class LightType : int
{
    Point       = 0,
    Directional = 1,
    Spot        = 2
};

static inline const char* lightTypeName(LightType t)
{
    switch (t)
    {
        case LightType::Point:       return "Point";
        case LightType::Directional: return "Directional";
        case LightType::Spot:        return "Spot";
    }
    return "Unknown";
}




struct Light
{
    std::string name        = "Light";
    LightType   type        = LightType::Point;
    bool        enabled     = true;


    glm::vec3   color       = glm::vec3(1.0f);
    float       intensity   = 1.0f;


    glm::vec3   position    = glm::vec3(0.0f, 3.0f, 0.0f);


    glm::vec3   direction   = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f));


    float       constant    = 1.0f;
    float       linear      = 0.09f;
    float       quadratic   = 0.032f;


    float       innerCutoff = 12.5f;
    float       outerCutoff = 17.5f;



    bool        editorSelected = false;
};




class LightManager
{
public:

    explicit LightManager(const std::string& jsonPath = "lights.json");


    void load();
    void save() const;


    int  addLight(const Light& light = Light{});
    void removeLight(int index);
    void duplicateLight(int index);

    Light&       get(int index)       { return lights[index]; }
    const Light& get(int index) const { return lights[index]; }
    int          count()        const { return static_cast<int>(lights.size()); }



    void uploadToShader(Shader& shader) const;


    std::vector<Light> lights;

private:
    std::string m_jsonPath;
};

#endif