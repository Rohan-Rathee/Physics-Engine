# Physics Engine

### A C++ OpenGL game engine Built in pure CPP, using GLFW, ASSIMP, Bullet Physics

For the purpose of simplicity, i'll use .cpp and ignore .h (with defnitions), unless explicitly i mention something. and i am new to all this readme creating and stuff, so forgive me if i go too deep, or too basic. Before the first refactor, all my game logic sat in a single file, no longer the case after implementing physics, or even ig model loader (a little after its creation actually)

The starting file is main.cpp, and calls upon 2 files, engine and game. This fairly recent change(in july itself) has been implemented to separate the engine and game logic, so as to make the engine able to run and create any game with minimal changes. The engine is constructed, then the game object, followed by initialization of the engine with the game as a parameter, and is then run, fairly basic stuff, and will omit such obvious file definitions from now on.

## The Engine

So, the main orchestrator is the Engine.cpp, which is, as on 10th july, has been refactored with 4 main functions, the constructor, destructor, initialize to handle everything the constructor can't as it must construct some stuff before others to pass them on as parameters, and the main run() loop.

Files that engine has direct ownership of are all the different systems, from the window system to the time manager, render, camera, scene and physics. then initialize takes the game as gives each system the pointers it requires from other files and functions.

```
 void Engine::run()
{
    while (!windowSystem->shouldClose() && running)
    {
        //Time Tick using Time manager
  
        //Input handling by calling the input buffers from the Input System

        //Physics sim steps forward, preceeded and suceeded by the scene's pre
        // and post physics updates, where prephysics handles the character game
        // logic like grounded and stuff, while post physics handles animation setting

        Character* camTarget = game ? game->getCameraTarget() : nullptr;
        if (camTarget && !inputSystem->isSpectatorMode())
            cameraRig.update(*camera, *camTarget, deltaTime);

        if (game) game->onUpdate(deltaTime);

        auto t2 = std::chrono::high_resolution_clock::now();
        renderSystem->getModelLoader()->updateAnimations(deltaTime);

        auto t3 = std::chrono::high_resolution_clock::now();
        scene->update(deltaTime);

        int windowWidth, windowHeight;
        glfwGetWindowSize(windowSystem->getGLFWWindow(), &windowWidth, &windowHeight);
        screenWidth  = static_cast<unsigned int>(windowWidth);
        screenHeight = static_cast<unsigned int>(windowHeight);
        glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom),
            (float)screenWidth / (float)screenHeight, 0.1f, 1000000.0f);
        glm::mat4 view = camera->GetViewMatrix();

        imguiSystem->beginFrame();
        if (game) game->onRenderUI();

        renderSystem->setScreenSize(screenWidth, screenHeight);
        renderSystem->render(*camera, currentTime, view, projection);
        auto t4 = std::chrono::high_resolution_clock::now();

        float inputMs   = std::chrono::duration<float, std::milli>(t1 - t0).count();
        float physicsMs = std::chrono::duration<float, std::milli>(t2 - t1).count();
        float animMs    = std::chrono::duration<float, std::milli>(t3 - t2).count();
        float renderMs  = std::chrono::duration<float, std::milli>(t4 - t3).count();
        float totalMs   = inputMs + physicsMs + animMs + renderMs;

        ImGui::Begin("Engine Profiler"); 
        ImGui::Text("Input:      %.2f ms", inputMs);
        ImGui::Text("Physics:    %.2f ms", physicsMs);
        ImGui::Text("Animation:  %.2f ms", animMs);
        ImGui::Text("Render:     %.2f ms", renderMs);
        ImGui::Text("Measured:   %.2f ms (%.0f fps)", totalMs, totalMs > 0.0f ? 1000.0f / totalMs : 0.0f);
        ImGui::End();
        imguiSystem->render();

        windowSystem->swapBuffers();
        windowSystem->pollEvents();

        static bool lastF1 = false, cursorVisible = false;
        bool f1 = glfwGetKey(windowSystem->getGLFWWindow(), GLFW_KEY_F1) == GLFW_PRESS;
        if (f1 && !lastF1)
        {
            cursorVisible = !cursorVisible;
            glfwSetInputMode(windowSystem->getGLFWWindow(), GLFW_CURSOR,
                cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        lastF1 = f1;
    }
}
```

Some inspirations for the project and engine architecture were taken from the following sources:

- https://learnopengl.com/ (for OpenGL rendering and shader management)
- ogldev.com (for OpenGL rendering and shader management)
- lowlevelgamedev (moral support)

LOST HOURS LOG (not visible in any file anymore but still necessary for my journey and learning process)
--------------------------------------------------------------------------------------------------------

Hr 0 onwards, build started  (note render engine camera window transform, scene and entity management, physics, input, and time management were all in this at this point)

Hr 3 first triangle rendered
Hr 6 first 3d cube
Hr 10 first 3d model loaded and rendered
Hr 12 first 3d model with texture
Hr 15 tried instancing, was not really neaded partially removed
Hr 18 implemented height map import (still has an exe on github) but not really needed removed, created terrain from actual himalyan ranges and coloured it procedurally accourding to slope, amazing stuff imo
Hr 18 going to refactor the engine into a more modular architecture with systems and utils, and a core engine class to manage them all. ps first of 3 refactors
Hr 21 onwards the engine started to resemble this one, with the creating a new model loader to load models, rendersystem for all rendering and a base engine class keeping it all separate
Hr 21-31+
