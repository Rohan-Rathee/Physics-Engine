#include "light_manager.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader.h"

namespace {
    static std::string vec3Str(const glm::vec3 &v) {
        char buf[128];
        snprintf(buf, sizeof(buf), "[%.6f,%.6f,%.6f]", v.x, v.y, v.z);
        return buf;
    }

    static std::string lightToJson(const Light &l) {
        std::ostringstream o;
        o << "  {\n";
        o << "    \"name\": \"" << l.name << "\",\n";
        o << "    \"type\": " << (int)l.type << ",\n";
        o << "    \"enabled\": " << (l.enabled ? "true" : "false") << ",\n";
        o << "    \"color\": " << vec3Str(l.color) << ",\n";
        o << "    \"intensity\": " << l.intensity << ",\n";
        o << "    \"position\": " << vec3Str(l.position) << ",\n";
        o << "    \"direction\": " << vec3Str(l.direction) << ",\n";
        o << "    \"constant\": " << l.constant << ",\n";
        o << "    \"linear\": " << l.linear << ",\n";
        o << "    \"quadratic\": " << l.quadratic << ",\n";
        o << "    \"innerCutoff\": " << l.innerCutoff << ",\n";
        o << "    \"outerCutoff\": " << l.outerCutoff << "\n";
        o << "  }";

        return o.str();
    }

    static void skipWS(const char *&p) {
        while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == ',' || *p == ':'))
            ++p;
    }

    static std::string readString(const char *&p) {
        if (*p != '"')
            return {};
        ++p;
        std::string s;
        while (*p && *p != '"') {
            if (*p != '\\')
                s += *p;
            ++p;
        }
        if (*p == '"')
            ++p;
        return s;
    }

    static float readFloat(const char *&p) {
        skipWS(p);
        char *end = nullptr;
        float v = (float)strtod(p, &end);
        if (end && end != p)
            p = end;

        return v;
    }

    static bool readBool(const char *&p) {
        skipWS(p);
        if (strncmp(p, "true", 4) == 0) {
            p += 4;
            return true;
        }
        if (strncmp(p, "false", 5) == 0) {
            p += 5;
            return false;
        }
        return false;
    }

    static glm::vec3 readVec3(const char *&p) {
        skipWS(p);
        if (*p == '[')
            ++p;
        float x = readFloat(p);
        if (*p == ',')
            ++p;
        float y = readFloat(p);
        if (*p == ',')
            ++p;
        float z = readFloat(p);
        skipWS(p);
        if (*p == ']')
            ++p;
        return glm::vec3(x, y, z);
    }

    static bool parseLight(const char *&p, Light &out) {

        skipWS(p);
        if (*p != '{')
            return false;
        ++p;

        while (*p && *p != '}') {
            skipWS(p);
            if (*p != '"') {
                ++p;
                continue;
            }
            std::string key = readString(p);
            skipWS(p);

            if (key == "name")
                out.name = readString(p);

            else if (key == "type")
                out.type = (LightType)(int)readFloat(p);

            else if (key == "enabled")
                out.enabled = readBool(p);

            else if (key == "color")
                out.color = readVec3(p);

            else if (key == "intensity")
                out.intensity = readFloat(p);

            else if (key == "position")
                out.position = readVec3(p);

            else if (key == "direction")
                out.direction = readVec3(p);

            else if (key == "constant")
                out.constant = readFloat(p);

            else if (key == "linear")
                out.linear = readFloat(p);

            else if (key == "quadratic")
                out.quadratic = readFloat(p);

            else if (key == "innerCutoff")
                out.innerCutoff = readFloat(p);

            else if (key == "outerCutoff")
                out.outerCutoff = readFloat(p);

            else {
                skipWS(p);
                if (*p == '"')
                    readString(p);

                else if (*p == '[') {
                    while (*p && *p != ']')
                        ++p;
                    if (*p)
                        ++p;
                } else {
                    while (*p && *p != ',' && *p != '}')
                        ++p;
                }
            }
            skipWS(p);
        }
        if (*p == '}')
            ++p;
        return true;
    }
}

LightManager::LightManager(const std::string &jsonPath)

    : m_jsonPath(jsonPath) {
    load();
    if (lights.empty())

    {
        Light sun;
        sun.name = "Sun";
        sun.type = LightType::Directional;
        sun.color = glm::vec3(1.0f, 0.95f, 0.80f);
        sun.intensity = 3.0f;
        sun.direction = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f));

        lights.push_back(sun);

        save();
    }
}

void LightManager::load() {
    std::ifstream f(m_jsonPath);
    if (!f.is_open())
        return;

    std::string src((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());

    const char *p = src.c_str();
    while (*p && *p != '[')
        ++p;

    if (*p != '[')
        return;

    ++p;

    lights.clear();
    while (*p) {
        while (*p && *p != '{' && *p != ']')
            ++p;

        if (!*p || *p == ']')
            break;

        Light l;
        if (parseLight(p, l))

            lights.push_back(l);
    }
    std::cout << "[LightManager] Loaded " << lights.size()
              << " light(s) from " << m_jsonPath << "\n";
}

void LightManager::save() const {

    std::ofstream f(m_jsonPath);

    if (!f.is_open()) {
        std::cerr << "[LightManager] Cannot write " << m_jsonPath << "\n";
        return;
    }

    f << "[\n";
    for (int i = 0; i < (int)lights.size(); ++i) {

        f << lightToJson(lights[i]);
        if (i + 1 < (int)lights.size())
            f << ",";

        f << "\n";
    }

    f << "]\n";
    std::cout << "[LightManager] Saved " << lights.size()

              << " light(s) to " << m_jsonPath << "\n";
}

int LightManager::addLight(const Light &light) {

    if ((int)lights.size() >= MAX_LIGHTS) {
        std::cerr << "[LightManager] MAX_LIGHTS (" << MAX_LIGHTS

                  << ") reached, cannot add more.\n";
        return -1;
    }

    lights.push_back(light);
    return static_cast<int>(lights.size()) - 1;
}

void LightManager::removeLight(int index) {

    if (index < 0 || index >= (int)lights.size())
        return;
    lights.erase(lights.begin() + index);
}

void LightManager::duplicateLight(int index) {

    if (index < 0 || index >= (int)lights.size())
        return;

    if ((int)lights.size() >= MAX_LIGHTS)
        return;

    Light copy = lights[index];

    copy.name += " (copy)";

    lights.insert(lights.begin() + index + 1, copy);
}

void LightManager::uploadToShader(Shader &shader) const {

    int n = 0;

    for (const auto &l : lights) {
        if (!l.enabled)
            continue;
        if (n >= MAX_LIGHTS)
            break;

        const std::string base = "u_lights[" + std::to_string(n) + "].";
        shader.setInt(base + "type", (int)l.type);
        shader.setVec3(base + "color", l.color * l.intensity);
        shader.setVec3(base + "position", l.position);
        shader.setVec3(base + "direction", glm::normalize(l.direction));
        shader.setFloat(base + "constant", l.constant);
        shader.setFloat(base + "linear", l.linear);
        shader.setFloat(base + "quadratic", l.quadratic);
        shader.setFloat(base + "innerCutoff", std::cos(glm::radians(l.innerCutoff)));
        shader.setFloat(base + "outerCutoff", std::cos(glm::radians(l.outerCutoff)));

        ++n;
    }
    shader.setInt("u_numLights", n);
}

void LightManager::loadFromFile(const std::string &jsonPath) {
    m_jsonPath = jsonPath;
    load();

    if (lights.empty()) {
        Light sun;
        sun.name = "Sun";
        sun.type = LightType::Directional;
        sun.color = glm::vec3(1.0f, 0.95f, 0.80f);
        sun.intensity = 3.0f;
        sun.direction = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.3f));
        lights.push_back(sun);
        save();
    }
}