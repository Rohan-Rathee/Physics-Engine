#ifndef BULLET_DEBUG_DRAWER_H 

#define BULLET_DEBUG_DRAWER_H 

#include <btBulletDynamicsCommon.h> 

#include <glm/glm.hpp> 

class Shader; 

class Shader; 

class BulletDebugDrawer : public btIDebugDraw 

{ 

public: 

    BulletDebugDrawer(); 

    ~BulletDebugDrawer(); 

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override; 

    void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override; 

    void reportErrorWarning(const char* warningString) override; 

    void draw3dText(const btVector3& location, const char* textString) override; 

    void setDebugMode(int debugMode) override; 

    int getDebugMode() const override { return m_debugMode; } 

    void initBuffers(); 

    void setShaderAndMatrices(Shader* shader, const glm::mat4& view, const glm::mat4& proj); 

private: 

    int m_debugMode; 

    unsigned int VAO, VBO; 

    bool initialized; 

    Shader* currentShader; 

    glm::mat4 viewMatrix, projMatrix; 

}; 

#endif 

