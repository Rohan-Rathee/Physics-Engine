#ifndef IMGUI_SYSTEM_H 
#define IMGUI_SYSTEM_H 
#include <imgui.h> 
#include <imgui_impl_glfw.h> 
#include <imgui_impl_opengl3.h> 
class GLFWwindow; 
class ImGuiSystem { 
public: 
     
    ImGuiSystem() = default; 
    ~ImGuiSystem() = default; 
    bool initialize(GLFWwindow* window); 
    void beginFrame(); 
    void render(); 
    void shutdown(); 
    bool showDemoWindow = false; 
private: 
    bool initialized = false; 
}; 
#endif