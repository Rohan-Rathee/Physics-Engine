#include "bullet_debug_drawer.h" 
#include "../shader.h" 
#include <iostream> 
#include <GL/gl.h> 
BulletDebugDrawer::BulletDebugDrawer() 
    : m_debugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawAabb), 
      VAO(0), VBO(0), initialized(false), currentShader(nullptr), 
      viewMatrix(1.0f), projMatrix(1.0f) 
{ 
      
} 
BulletDebugDrawer::~BulletDebugDrawer() 
{ 
    if (VBO != 0) 
        glDeleteBuffers(1, &VBO); 
    if (VAO != 0) 
        glDeleteVertexArrays(1, &VAO); 
} 
void BulletDebugDrawer::initBuffers() 
{ 
    if (initialized) 
        return; 
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO); 
     
    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, 1000 * sizeof(float), nullptr, GL_DYNAMIC_DRAW); 
     
    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); 
     
    glEnableVertexAttribArray(1); 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); 
     
    glBindVertexArray(0); 
    initialized = true; 
} 
void BulletDebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) 
{ 
    if (!initialized || !currentShader) 
        return; 
    float vertices[] = { 
        from.x(), from.y(), from.z(), color.x(), color.y(), color.z(), 
        to.x(),   to.y(),   to.z(),   color.x(), color.y(), color.z() 
    }; 
     
    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
    glDrawArrays(GL_LINES, 0, 2); 
} 
void BulletDebugDrawer::setShaderAndMatrices(Shader* shader, const glm::mat4& view, const glm::mat4& proj) 
{ 
    currentShader = shader; 
    viewMatrix = view; 
    projMatrix = proj; 
    if (currentShader) 
    { 
        currentShader->use(); 
        currentShader->setMat4("view", viewMatrix); 
        currentShader->setMat4("projection", projMatrix); 
    } 
} 
void BulletDebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,  
                                         btScalar distance, int lifeTime, const btVector3& color) 
{ 
    btVector3 to = PointOnB + normalOnB * distance; 
    drawLine(PointOnB, to, color); 
} 
void BulletDebugDrawer::reportErrorWarning(const char* warningString) 
{ 
    std::cerr << "Bullet Warning: " << warningString << std::endl; 
} 
void BulletDebugDrawer::draw3dText(const btVector3& location, const char* textString) 
{ 
      
} 
void BulletDebugDrawer::setDebugMode(int debugMode) 
{ 
    m_debugMode = debugMode; 
} 
