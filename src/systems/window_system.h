#ifndef WINDOW_SYSTEM_H
#define WINDOW_SYSTEM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class WindowSystem {
private:
    GLFWwindow* window;
    unsigned int width, height;
    std::string title;

public:
    WindowSystem(unsigned int w, unsigned int h, const std::string& t);
    ~WindowSystem();

    bool initialize();
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void close();
    
    GLFWwindow* getGLFWWindow() const { return window; }
    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }
    void setWidth(unsigned int w) { width = w; }
    void setHeight(unsigned int h) { height = h; }
};

#endif
