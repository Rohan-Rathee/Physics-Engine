/**
 * @file imgui_system.h
 * @brief Manages the engine's Dear ImGui interface.
 *
 * Provides editor and debugging tools built with Dear ImGui, as well as interface to alter and modify textures and lights run time
 *
 * Features include:
 * - Light editor.
 * - Model inspector.
 * - Material editor.
 * - Player HUD.
 * - Bot health bars.
 *
 * ------------------------
 * TODO: offload to a better library or eventually a custom GUI system.
 * ------------------------
 */

#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "../utils/light_manager.h"
#include "../utils/model_loader.h"  

class GLFWwindow;
class Character;

class ImGuiSystem
{
public:

    ImGuiSystem() = default;
    ~ImGuiSystem() = default;
  
    bool initialize(GLFWwindow* window);   
    void setLightManager(LightManager* lm) { m_lightManager = lm; }
    void setModelLoader(ModelLoader* ml)   { m_modelLoader  = ml; }  
    void beginFrame();
    void render();
    void shutdown();
    bool showDemoWindow = false;
    void renderPlayerHUD(const Character* player);

    void renderBotHealthBars(const std::vector<std::shared_ptr<Character>>& bots, const glm::mat4& view, const glm::mat4& projection,unsigned int screenWidth,unsigned int screenHeight);

    void onLevelChanged();
private:
        
    bool worldToScreen(const glm::vec3& worldPos,const glm::mat4& viewProj,unsigned int screenW, unsigned int screenH,glm::vec2& out);

    void renderLightEditor();
    void renderModelInspector();                              
    void renderMaterialEditor(PBRMaterial& mat, const char*  meshNodeName);

    bool          initialized    = false;
    LightManager* m_lightManager = nullptr;
    ModelLoader*  m_modelLoader  = nullptr;  

    int  m_selectedLight = -1;
    bool m_addMenuOpen   = false;
    int  m_selectedModel = -1;
    int  m_selectedMesh  = -1;
};


