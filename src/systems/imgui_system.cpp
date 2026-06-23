#include "imgui_system.h" 
bool ImGuiSystem::initialize(GLFWwindow* window) 
{ 
    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext(); 
    ImGuiIO& io = ImGui::GetIO(); 
    (void)io; 
    ImGui::StyleColorsDark(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init("#version 330"); 
    initialized = true; 
    return true; 
} 
void ImGuiSystem::beginFrame() 
{ 
    ImGui_ImplOpenGL3_NewFrame(); 
    ImGui_ImplGlfw_NewFrame(); 
    ImGui::NewFrame(); 
    ImGui::Begin("Engine Debug"); 
    ImGui::Text("Physics Engine"); 
    ImGui::Separator(); 
    ImGui::Checkbox("Demo Window", &showDemoWindow); 
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate); 
    ImGui::End(); 
    if (showDemoWindow) 
        ImGui::ShowDemoWindow(&showDemoWindow); 
} 
void ImGuiSystem::render() 
{ 
    ImGui::Render(); 
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 
} 
void ImGuiSystem::shutdown() 
{ 
    if (!initialized) 
        return; 
    ImGui_ImplOpenGL3_Shutdown(); 
    ImGui_ImplGlfw_Shutdown(); 
    ImGui::DestroyContext(); 
    initialized = false; 
}