#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

class TimeManager {
private:
    float deltaTime;
    float lastFrame;
    float currentFrame;

public:
    TimeManager();
    
    void update();
    float getDeltaTime() const { return deltaTime; }
    float getCurrentTime() const { return currentFrame; }
    float getFPS() const;
};

#endif
