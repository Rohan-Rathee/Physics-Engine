# Totally <sub><sup><sub><sup><sub>in</sup></sup></sub></sup></sub>Accurate Game Simulator


### A C++ OpenGL game engine Built in pure CPP, using GLFW, ASSIMP, Bullet Physics

Before diving in, a small note: throughout this README I'll primarily refer to the `.cpp` files and omit the corresponding `.h` files unless they're directly relevant. Most of the interesting implementation lives in the source files, so that keeps things a little easier to follow.This is also my first time writing a project README of this scale, so forgive me if I occasionally go into more detail than necessary, or explain something that's already obvious. 

Before the first refactor, all my game logic sat in a single file, no longer the case after implementing physics, or even ig model loader (a little after its creation actually)

The starting file is `main.cpp`, and it calls upon two files, engine and game. This fairly recent change(in july itself) has been implemented to separate the engine and game logic, so as to make the engine able to run and create any game with minimal changes. The engine is constructed, then the game object, followed by initialization of the engine with the game as a parameter, and is then run, fairly basic stuff, and will omit such obvious file definitions from now on.


## Features

- Modern OpenGL 3.3 Core Renderer
- Physically Based Rendering (Cook-Torrance BRDF)
- HDR Rendering & Tone Mapping
- Image-Based Lighting (IBL)
- Bloom Post-Processing
- Shadow Mapping
- Skeletal Animation & Blend Trees
- Bullet Physics Integration
- Fixed Timestep Physics Simulation
- Third-Person & Free-Fly Cameras
- Character Controller Framework
- AI Controller Framework
- ASSIMP Model Import (GLTF, GLB, FBX, OBJ)
- Automatic Collision Shape Generation
- Runtime Material & Lighting Editor (ImGui)
- JSON Scene & Light Serialization
- Multi-Pass Rendering Pipeline
- Modular Engine/System Architecture
- Engine/Game Separation
- Built Completely from Scratch in Modern C++

# Contents

- Engine
- Renderer
  - Rendering Pipeline
  - ASSIMP Pipeline
  - OpenGL Pipeline
  - Entering 3D
  - PBR
- Physics
- Camera
- Input System
- ImGui
- Characters
- Animation
- Inspirations
- Lost Hours Log

## Architecture Overview

```
main.cpp
     │
     ▼
 Engine
     │
     ├────────────┐
     ▼            ▼
 Renderer      Physics
     │            │
     ▼            ▼
 Scene       Character
     │
     ▼
 ImGui
```


## The Engine

We must have an order, so lets start from the most impactful one on my journey to the least but still siginificant parts:

1. The Engine: Both the overall engine and the file engine
2. The Render and Model Loader: inter-twined beyond reason due to my sheer newbieness
3. The Inputs and Camera:
4. Physics
5. Animators
6. Character controller

Most others are utils and will be covered as and when used. shaders will be covered with renderer as and when called.

The main orchestrator is `Engine.cpp`, which, as of the 10th of July, has been refactored into four main functions: the constructor, the destructor, `initialize` (to handle everything the constructor can't, since some things must be constructed before others so they can be passed on as parameters), and the main `run()` loop.


Files that engine has direct ownership of are all the different systems, from the window system to the time manager, render, camera, scene and physics.  `initialize` then takes the game and gives each system the pointers it requires from other files and functions.

```cpp
void Engine::run()
{
    while (!windowSystem->shouldClose() && running)
    {
        // Time tick using the Time Manager. The chrono calls are separate from this and are
        // for handling time-sensitive physics and animations.

        // Input handling by polling the input buffers from the Input System.

        // Physics simulation steps forward, preceded and followed by the scene's pre-
        // and post-physics updates, where pre-physics handles character game logic
        // like grounded state, while post-physics handles setting animations.

        // Camera snapping to its respective target.

        // Game logic update, and sending animation settings to the Render System.

        // Render System renders it all.

        // ImGui System renders the game and editor GUI.

        // Window System handles buffer swapping and polling of events —
        // a fancy way of saying it takes what the renderer drew (back buffer) and
        // gives it to the monitor (front buffer), then handles input polling.
        // One thing to note: input polling happens last, while input handling happens
        // first. This isn't wrong or right, because run() is a loop and the variables
        // and objects are preserved — input -> physics -> render -> poll is equivalent
        // to poll -> input -> render -> physics.

        // Editing utilities, like toggling the cursor with F1.
    }
}
```

### The Renderer

The Renderer is the largest subsystem in the engine, and also the one that has gone through the most rewrites. It started as a single triangle renderer and gradually evolved into a multi-pass PBR renderer supporting HDR, bloom, shadows, and image-based lighting.

Unlike the rest of the engine, the Renderer is split into several rendering passes instead of simply drawing everything directly to the screen. Each pass produces information that the following pass depends on, eventually resulting in the final image presented to the monitor. In effect, the same scene is currently rendered once for shadow mapping, then once for base rendering, followed by multiple ping-pong bloom iterations.

The rendering pipeline currently looks something like this:
```text
HDRI & Model Loading
      │
      ▼
IBL Precomputation
(Environment Cubemap, Irradiance, Prefilter, BRDF LUT)
      │
      ▼
Shadow Pass
      │
      ▼
Main PBR Render Pass
      │
      ▼
Gamma Correction
      │
      ▼
Bright Pixel Extraction
      │
      ▼
Gaussian Bloom Blur
      │
      ▼
HDR Composite
      │
      ▼
Tone Mapping
      │
      ▼
Back Buffer
```

Everything after the Shadow Pass runs every frame.

The pbr pass itself is made up of:

1. Material Maps inputed
2. Reconstruct Material (BaseColor, Normal, Metallic, Roughness, AO)
3. Cook-Torrance BRDF (GGX + Smith + Fresnel): uses 2
4. Direct Lights: uses 3
5. IBL: uses 3
6. Ambient + Specular: uses 5
7. Shadows //partially functioning to save performance
8. Emissive: uses 2
9. Fog: uses distance to camera
10. FragColor : uses 3,4,5,6,7,8,9, and outputs to back to the gpu
11. BrightColor (Bloom): separate pass using FragColor passed to gpu as input
12. Output: uses 10 and 11 (implemented in 11 itself) and results in the final render being displays to the screen


#### How It Works

*(Skip to the next section if you're familiar with how OpenGL rendering works.)*

Three pipelines feed the main render pipeline one after the other: the ASSIMP model processing pipeline, the OpenGL rendering pipeline, and the renderer pipeline I built on top of it. The flow is as follows:

```text
Model File (Blender, etc.)
      │
      ▼
ASSIMP Import Pipeline
      │
      ▼
CPU Mesh Objects
      │
      ▼
OpenGL Upload Pipeline (VBOs, VAOs, and per-frame transforms
from animation and physics)
      │
      ▼
GPU Buffers
      │
      ▼
Vertex Shader (per vertex)
      │
      ▼
Rasterizer (triangles → fragments)
      │
      ▼
Fragment Shader (per fragment)
      │
      ▼
Final Image
```

#### ASSIMP Pipeline

Handles asset importing and resides in `model_loader`.

It takes in any common 3d file format, GLB(inbuilt binary textures while rest have separate texture folders), GLTF, FBX, OBJ.

It walks down the scene, adds a model, adds a mesh within that model, and, iterating through the file, adds vertices within that mesh.

End result is you get all this during initilization itself:

Scene owns models, Models own collision shapes, meshes and bonemaps, meshes own a vector list of  PBR textures, a list of vertices, and later on, has somewhat of an ownership of the buffer and array objects. The vertex owns a lot of data, from positions and texture coordinates, to bone weights for animation.

model loader's load model:
```cpp
void Model::loadModel(const std::string &path) {
    importer = std::make_unique<Assimp::Importer>();
    unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
                         aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
    const aiScene *scene = importer->ReadFile(path, flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer->GetErrorString() << std::endl;
        return;
    }
    scene_ptr = scene;

    if (scene->HasAnimations()) {
        std::cout << "Animations: " << scene->mNumAnimations << std::endl;
    }
    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene, glm::mat4(1.0f));
    computeBounds();
}
```


Process node — FIFO node loading for individual objects:
```cpp
void Model::processNode(aiNode *node, const aiScene *scene, glm::mat4 parentTransform) {
    glm::mat4 transform = parentTransform * aiMatrix4x4ToGlm(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        MeshInstance instance;
        instance.mesh = processMesh(mesh, scene);
        instance.transform = transform;
        instance.name = node->mName.C_Str();
        meshes.push_back(instance);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, transform);
    }
}
```

Final process mesh: to process vertices one at a time
```cpp
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<texture> textures;

    // Vertex loop
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        vertex v;
        // Process the position and texture coordinates
        vertices.push_back(v);
    }

    // Tangent, bitangent, and bone system processing for animation

    // Texture map imports (currently called one at a time — a function or loop
    // implementation is pending)

    // Finally, process and set vertex and texture maps

    Mesh result_mesh(vertices, indices, textures);
    result_mesh.material.name = mesh->mName.C_Str();
    result_mesh.material.metallic = metallicFactor;
    result_mesh.material.roughness = roughnessFactor;
    result_mesh.material.baseColor = glm::vec3(1.0f);

    if (isRealTexture(emissiveMaps))
        result_mesh.material.emissive = glm::vec3(1.0f);
    else
        result_mesh.material.emissive = glm::vec3(
            emissiveFactor.r, emissiveFactor.g, emissiveFactor.b);
    result_mesh.material.emissiveIntensity = emissiveIntensity;

    result_mesh.material.hasNormalMap = isRealTexture(normalMaps);
    result_mesh.material.hasMetallicRoughnessMap = isRealTexture(mrMaps);
    result_mesh.material.hasAOMap = isRealTexture(aoMaps);
    result_mesh.material.hasEmissiveMap = isRealTexture(emissiveMaps);

    for (const auto &t : textures) {
        if (t.type == "diffuse")
            result_mesh.material.albedoTexID = t.id;
        else if (t.type == "normal")
            result_mesh.material.normalTexID = t.id;
        else if (t.type == "metallicRoughness")
            result_mesh.material.metallicRoughnessTexID = t.id;
        else if (t.type == "ao")
            result_mesh.material.aoTexID = t.id;
        else if (t.type == "emissive")
            result_mesh.material.emissiveTexID = t.id;
    }

    return result_mesh;
}
```


All of this lives in RAM as C++ objects, accessible by the CPU but not yet by the GPU.

#### OpenGL Pipeline

Responsible for transferring the model into GPU memory and eventually converting it into pixels.

This is where VAOs, VBOs, shaders and draw calls come into play. Most of it are standard calls present in any engine and are part of opengl. The rest, the shaders, the designing of what this pipelines does first, next, and carries over what, is the renderer pipeline.

The details of the OpenGL pipeline are well documented elsewhere, so this section focuses on how my engine interacts with it rather than OpenGL itself. Look into `renderPass` for the frame-by-frame setting of uniforms and the actual draw calls, and the Model Loader's three draw functions for the draw logic based on animation weights.

There, A VBO simply stores the raw vertex data inside GPU memory. Once uploaded, the mesh rarely needs to be sent again, allowing it to be rendered every frame with only a draw call.

The VAO doesn't store any vertex data itself. Instead, it remembers how the data inside one or more VBOs should be interpreted. For example, it tells OpenGL that the first three floats of every vertex represent its position, the next three represent the normal, followed by texture coordinates, tangents, bitangents, bone IDs and weights. EBO are also present, and work as a way to tell the gpu what all three vertices form a trangle

Once all this is done, rendering proceeds as follows: the renderer binds the mesh's VAO, applies all transformations along with the model, view, and projection matrices (these three are what make it appear 3D), binds the texture, and issues the draw call `glDrawElements`. OpenGL then executes the vertex shader for every vertex, assembles those vertices into triangles, rasterizes the triangles into fragments, and finally executes the fragment shader to determine the colour of every visible pixel.

one good example of this is the setup mesh from the modelloader

```cpp
void Mesh::setupMesh() {
    // The 3 objects:
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    // Binding of the data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Telling it how to read and pass the data to the shaders —
    // each one of these is passed from the VBO to the vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (void *)offsetof(vertex, normal));
    // ...
}
```

^^^ during initialization, followed by multiple `modelShader->set<type>(<>, ...)` calls every frame and a draw call.

The draw system lives neatly together in the Model Loader, and is overloaded to handle animations, colliders, ghosts, and general draw-time transform skipping.

#### Entering 3D

Before we enter the glorious PBR, every vertex has to go from a 3d coordinate to a actual point on the screen. And it can just use that coordinate and clip one of those, or we would get someting like an orthogonal perspective, and even after the perspective, we must cull sides based on different parameters(currently distance culling depth-test culling, and backface culling).


The renderer gets 3 matricies that help render the model, 4x4 matricies to accomodate all position rotation and scale in one matrix. All must have a correct order of setting ztranslate, then rotate, then scale.

The **Model Matrix** comes from the Model Loader and is a per-mesh-instance transform baked in while processing the node. It describes the transform of that specific model/instance.

The **View Matrix** comes from the camera — so technically, the camera never moves; everything else moves around the camera, in reverse of wherever the camera itself moves. It's common to all models and the entire scene.

The **Projection Matrix** is assembled every frame and doesn't care much about the camera except for its zoom (which actually stores FOV) — it's the part that adds a sense of 3D through depth. The near and far planes are intentionally much larger than normal. They are leftovers from an earlier version of the engine that rendered procedural Himalayan terrain generated from heightmaps. The terrain system has since been removed, but the projection settings remain mostly unchanged. Any performance gain from tightening these values would be minimal now, given distance culling and the relative simplicity of the current models compared to the old Himalayan heightmaps.

Both the view and projection matrices are computed once per frame in the engine loop and handed down into `renderSystem->render(*camera, currentTime, view, projection)`, from where they eventually reach the vertex shader as uniforms. Inside the shader itself, the actual "make it 3D" moment is just the standard `gl_Position = projection * view * model * vec4(position, 1.0)`.

#### PBR Renderer 

The main renderer uses a Cook-Torrance BRDF with the GGX microfacet model.
After recieving stuff as the objects, the control is passed on to the shaders.
first we have a minimal vertex shader controling animations, then to the fragments shader with actual Physically based rendering implementation.


PBR lives almost completely in the shaders, after the uniforms and layout have been set. PBR is a method of rendering that lets us give objects metallic, roughness, AO, and emissive traits, and make them look real.

The Model Loader extracts, in either map or constant form:
- Base Color
- Metallic
- Roughness
- Ambient Occlusion
- Emissive

Each fragment computes:
- Normal Distribution Function (GGX)
- Smith Geometry Function
- Schlick Fresnel Approximation


These terms are combined using the Cook-Torrance Bidirectional Reflectance Distribution Function (BRDF):

with the final fromula as $$L_o=\left(k_D\frac{\text{Albedo}}{\pi}+\frac{DGF}{4(N\cdot V)(N\cdot L)}\right)L_i(N\cdot L)$$

where:

- **D** is the **GGX Normal Distribution Function**, describing the statistical distribution of microscopic surface facets.
- **G** is the **Smith Geometry Function**, accounting for masking and shadowing between those microfacets.
- **F** is the **Schlick Fresnel Approximation**, describing how surface reflectivity changes with viewing angle.
- **$L_i$** is the incoming light radiance.
- **$L_o$** is the outgoing reflected radiance.
- **$N$** is the surface normal.
- **$V$** is the view direction.
- **$L$** is the light direction.

To conserve energy, the diffuse component is scaled by

$$
k_D=(1-F)(1-\text{Metallic})
$$

while the specular reflectance at normal incidence is computed as

$$
F_0=\text{mix}(0.04,\ \text{Albedo},\ \text{Metallic})
$$

This means dielectric materials retain both diffuse and specular reflections, whereas metallic materials gradually shift almost all reflected energy into the specular term. The shader computes **D**, **G**, and **F** independently before combining them into the Cook-Torrance BRDF, after which direct lighting, image-based lighting (IBL), shadows, ambient occlusion, emissive lighting, and fog are applied to produce the final fragment colour.

Earlier, I had implemented Phong lighting, but PBR was a day night difference after a proper implementation.

One would notice, we dont just have a single vertex and fragment shader pair, but multiple. this it due to the PBR requiring us to sample the environment for the reflective and/or brightning of the objects based on the HDRI environment that is in our skybox.

### The Physics

The Physics run on bullet, that has been wrapped in Physics System in a minimal way. It owns the 4 parts that make the bullet library funtion:
1. The ColliisionConfig
2. Dispatcher
3. broadpahse
4. Impulse based solver

All of this is tied together through the `dynamicsWorld`, which is the middleman between the system and the rest of the project.

```cpp
bool PhysicsSystem::initialize(const glm::vec3& gravityVec) {
    collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
    broadphase = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get()
    );
    setGravity(gravityVec);
    return true;
}
```

Since Phsysics behaves identically regardless of frametime, i have used an accumulator, which works by incrementing the accumulator the time passed since the last sim step, and the accumulator basically holds the unsimulated time.

```cpp
void PhysicsSystem::update(float deltaTime) {
    accumulator += deltaTime;
    while (accumulator >= fixedStep) {
        dynamicsWorld->stepSimulation(fixedStep, 0);
        accumulator -= fixedStep;
    }
}
```

As soon as the unsimulated time exceeds fixed step, the sim runs forward exactly the fixed step and decrements the acc by the fixed step, multiple times as long as the accumulated time still exceeds the fixed step. This ensures the sim is independent of the frame rate of the engine.

Rigid bodies are created via `createRigidBody`, taking in mass, a collision shape built by the Model Loader (convex hull, compound box, triangle mesh, or capsule, depending on what the model needs), a spawn position, and a restitution value. At zero mass, a body is static, with no inertia calculations performed. All other physics constants, like friction and damping, are hardcoded defaults for now.

```cpp
btRigidBody* PhysicsSystem::createRigidBody(float mass, btCollisionShape* shape,
                                             const glm::vec3& position, float restitution) {
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(position.x, position.y, position.z));

    btMotionState* motionState = new btDefaultMotionState(transform);
    btVector3 localInertia(0, 0, 0);
    if (mass != 0.0f) shape->calculateLocalInertia(mass, localInertia);

    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setRestitution(restitution);
    body->setFriction(0.5f);
    body->setRollingFriction(0.01f);
    body->setSpinningFriction(1.0f);
    body->setDamping(0.5f, 0.1f);

    dynamicsWorld->addRigidBody(body);
    return body;
}
```

Model Transform sits between the Physics System and the render side. Every frame, it pulls each body's transform, converts the quaternion into an angle/axis pair, and hands it off to the Render System.


```cpp
void ModelTransform::applyPhysicsTransform(const PhysicsModelData &physicsModel) {
    btTransform btTrans;
    physicsModel.rigidBody->getMotionState()->getWorldTransform(btTrans);
    // ... quaternion -> angle/axis conversion ...
    setTransform(physicsModel.modelIndex, position, scale, angle, axis);
}
```

The characters bypass modelTransform for their movement, and are only concerned with collsions, so a direct kinematic method of setting velocity directly when grounded and regular phyics when in air is utilised, as well as angular-velocity-based turning toward the aim direction rather than snapping rotation instantly. Shooting is also just a physics raycast (`rayTest` from muzzle to max range), with the hit rigid body's user pointer cast back to a `Character*` to apply damage directly — this is why every rigid body gets `setUserPointer(this)` in the constructor.

### Input and Camera

#### Camera
`camera.h` is the oldest file in the project, and while everything else has been rewritten multiple times, this and `shader.h` have remained mostly untouched. It's the classic LearnOpenGL free-fly camera, written while I was still learning and experimenting with OpenGL — working with only Euler angles (Yaw and Pitch), with no quaternions or roll.

```cpp
void updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}
```

Every rotation, either by direct mouse look or by a snapping to 3rd person snapping, works by recomputing front up and right from Yaw and pitch, and then making the perspective matrix from that.

#### Third person camera rig

`ThirdPersonCameraRig` is completely separated from the camera, and the camera itself is kept "dumb." The rig converts the camera's own Yaw/Pitch into an orbit direction, then places the camera at `followDistance` back from a point `followHeight` above the target:

```cpp
glm::vec3 orbitDir(
    cos(yawRad) * cos(pitchRad),
    sin(pitchRad),
    sin(yawRad) * cos(pitchRad)
);
glm::vec3 targetPoint = modelPos + vehicleUp * followHeight;
glm::vec3 desiredPos  = targetPoint - orbitDir * followDistance;
```

postion and facing the both eased towards the target instead of snapped, but is currently almost instant. First it computes the front direction, then manually rederives Right and Up, and writes Pitch back from `asin(Front.y)` to prevent drift, since only Front (rather than the full `updateCameraVectors()` path) is being driven directly here.

This only runs when `!inputSystem->isSpectatorMode()` and a camera target exists (see `Engine::run()`), so spectator mode and third-person follow take turns owning the camera and avoid conflict.

#### Input system

Like `camera.h`, the Input System is also kept dumb, and exists only as a hardware-to-signal translation layer, with `jumpPressed` and `firePressed` kept only as a sort of toggle passthrough.

Every callback forwards to ImGui first (`ImGui_ImplGlfw_...Callback`) before checking `io.WantCaptureMouse`/`WantCaptureKeyboard` and bailing if ImGui wants the input, so the editor UI always gets first refusal on clicks and drags.

```cpp
void InputSystem::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (ImGui::GetIO().WantCaptureMouse) return;
    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) return;
    // ...
}
```

The F1 toggle in `Engine::run()` also disables the cursor and rotation handling when interacting with ImGui, completely bypassing input capture for it.

### The UI

Well, curerntly its imgui with later plans to implement a renderer based Custom GUI.

InGui system kinda wrap the standard GLFWand opengl Imgui backed init, so opengl renders it all in-window.

```cpp
bool ImGuiSystem::initialize(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");
    initialized = true;
    return true;
}
```
ImGui itself doesn't install its own GLFW callbacks, but instead relies on those of the Input System to eliminate input fighting over the same callbacks, and input system is the single GLFW point of  contact

The per-frame entry point is `beginFrame`, drawing the small debug window first, followed by calls to the two big editor panels:

```cpp
void ImGuiSystem::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Engine Debug");
    ImGui::Checkbox("Demo Window", &showDemoWindow);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();
    if (showDemoWindow) ImGui::ShowDemoWindow(&showDemoWindow);
    renderLightEditor();
    renderModelInspector();
}
```

#### The Light Editor

A 2 pane layout with log based intensity control. Supports sun, point and cone based lighting, as well as attenuation for each(effective range and how they dim during travel).

All added lights get saved to a json file for the appropriate level, based on dirty flag, checked at end to call a save funtion.

The lights all get passed first into the renderer and shaders, allowing PBR to implement at most 16 independent lights.

#### Model Inspector

Similar to the lights, a simple model inspector has also been implemented. `renderMaterialEditor` is a straightforward per-channel breakdown — Albedo, Normal, Metallic/Roughness, AO, Emissive, each with its own thumbnail. Allows overwriting constant values.

### The Characters and animation and controller

These three pieces form one pipeline: a controller decides what a character wants to do, Character turns that into physics and a target animation blend, and the animator/bone system is what actually produces the skinned pose the renderer draws. Worth reading as one block since the last stage only exists to serve the first two.

Every input source implements the same interface regardless of human or ai nature of the inputs, allowing the charater to be completely agnostic about the driving force


```cpp
struct ControlInput {
    glm::vec3 moveDir{0.0f};
    glm::vec3 aimDir{0.0f, 0.0f, -1.0f};
    bool jumpPressed = false;
    bool firePressed = false;
};
class IController {
public:
    virtual ControlInput getInput(float deltaTime, const glm::vec3& currentPosition) = 0;
};
```


`moveDir` and `aimDir` are kept separate to allow for strafing — human keyboard input drives `moveDir`, while the mouse drives `aimDir`.

The AI Controller is currently a plain patrol-or-chase state: walk a waypoint list until a target enters `chaseRange`, then walk straight at it, with no pathing.

Chess piece controller is a demo of more movement types without full pathing, rook and bishops axis and diagonal snapping, knights 2 phase L movement, qweens free dashes, and king and pawns small dashes. Also, pawns try to surround you.

#### Character: turning input into physics + a target blend

`Character::prePhysicsUpdate` is the one place all of this meets:

```cpp
ControlInput input = controller->getInput(deltaTime, getPosition());
syncGroundedState();
applyMovement(input, deltaTime);
applyFacing(input, deltaTime);
if (hasShootAnim) {
    if (input.firePressed) shootTimer = shootAnimDuration;
    else shootTimer = glm::max(shootTimer - deltaTime, 0.0f);
}
handleShooting(input, deltaTime);
```


Movement blends horizontal velocity toward `moveDir * moveSpeed` while grounded, and applies raw force while airborne; facing is turned by applying angular velocity. Shooting is hitscanned against the physics rigid bodies, with a pointer sent back to apply damage directly.

The output that matters for animation is `updateAnimationBlend()`, called form `postPhysicsUpdate` right after `syncRenderTransform()`:

```cpp
float speed = glm::length(glm::vec2(velocity.x(), velocity.z()));
float runWeight = glm::clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0f, 1.0f);
if (hasShootAnim) {
    float shootWeight = glm::clamp(shootTimer / shootAnimDuration, 0.0f, 1.0f);
    float moveWeight  = 1.0f - shootWeight;
    modelLoader->setBlendWeights(modelIndex, {runWeight * moveWeight, (1.0f - runWeight) * moveWeight, shootWeight});
} else {
    modelLoader->setBlendWeights(modelIndex, {runWeight, 1.0f - runWeight});
}
```

`setBlendWeights` writes straight into an `Animator`'s `BlendLayer` list.

#### Bone: keyframes

`Bone` holds each animated node's raw position/rotation/scale keyframes from Assimp, and `Update(animTime)` composes them as `translation * rotation * scale`:

```cpp
glm::mat4 translation = InterpolatePosition(animationTime);
glm::mat4 rotation    = InterpolateRotation(animationTime);
glm::mat4 scale       = InterpolateScaling(animationTime);
localTransform = translation * rotation * scale;
```

Rotation is handled by slerp interpolation, and the rest use `glm::mix`. The keyframe-index lookup caches the last index found (`m_LastPosIdx`, etc.) rather than searching from scratch each time.

## Inspirations

Some inspirations for the project and engine architecture were taken from the following sources:

- [learnopengl.com](https://learnopengl.com/) (for OpenGL rendering and shader management)
- ogldev.com (for OpenGL rendering and shader management)
- lowlevelgamedev (moral support)

## Closing Thoughts

This project started as a way to learn modern OpenGL and slowly evolved into a complete game engine. Every subsystem has been rewritten multiple times, and many features that no longer exist were still valuable learning experiences.

The engine is still under active development, and there are plenty of things I'd like to improve. The list below is only the beginning.

## Future Work

- An actual game implementation
- First-person camera with locked player model
- ECS
- Deferred Renderer
- Better Animation Graph
- Audio Engine
- Networking
- Vulkan Backend

## Lost Hours Log

*(Not visible in any file anymore, but still necessary for my journey and learning process.)*



Build started — render engine, camera, window, transform, scene and entity management, physics, input, and time management were all in one file at this point.

- **0 h** — Started the project.
- **3 h** — First triangle rendered.
- **6 h** — First 3D cube.
- **10 h** — First 3D model loaded and rendered.
- **12 h** — First 3D model with texture.
- **15 h** — Experimented with instancing; wasn't really needed, partially removed.
- **18 h** — Implemented heightmap import (still has an exe on GitHub, but not really needed, since removed). Created terrain from actual Himalayan ranges and coloured it procedurally according to slope — amazing stuff, in my opinion.
- **18 h** — Began refactoring the engine into a more modular architecture, with systems, utils, and a core Engine class to manage it all. (First of three refactors.)
- **21 h+** — The engine started to resemble its current form: a new Model Loader for loading models, a Render System for all rendering, and a base Engine class keeping it all separate.
