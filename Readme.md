# Totally <sub><sup><sub><sup><sub>in</sup></sup></sub></sup></sub>Accurate Game Simulator

### A C++ OpenGL game engine Built in pure CPP, using GLFW, ASSIMP, Bullet Physics

Ok, sooo, i'll be use .cpp and ignore .h (with defnitions), unless explicitly i mention something. and I am new to all this readme creating and stuff, so forgive me if i go too deep, or too basic. Before the first refactor, all my game logic sat in a single file, no longer the case after implementing physics, or even ig model loader (a little after its creation actually)

The starting file is main.cpp, and calls upon 2 files, engine and game. This fairly recent change(in july itself) has been implemented to separate the engine and game logic, so as to make the engine able to run and create any game with minimal changes. The engine is constructed, then the game object, followed by initialization of the engine with the game as a parameter, and is then run, fairly basic stuff, and will omit such obvious file definitions from now on.

## The Engine

We must have an order, so lets start from the most impactful one on my journey to the least but still siginificant parts:

1. The Engine: Both the overall engine and the file engine
2. The Render and Model Loader: inter-twined beyond reason due to my sheer newbieness
3. The Inputs and Camera:
4. Physics
5. Animators
6. Character controller

most others are utils and will be covered as and when used. shaders will be covered with renderer as and when called.

So, the main orchestrator is the Engine.cpp, which is, as on 10th july, has been refactored with 4 main functions, the constructor, destructor, initialize to handle everything the constructor can't as it must construct some stuff before others to pass them on as parameters, and the main run() loop.

Files that engine has direct ownership of are all the different systems, from the window system to the time manager, render, camera, scene and physics. then initialize takes the game as gives each system the pointers it requires from other files and functions.

```
void Engine::run()
{
    while (!windowSystem->shouldClose() && running)
    {
        //Time Tick using Time manager, the Chrono calls are separate from this and are for 
        // handling time sensitive physics and animations 
  
        //Input handling by calling the input buffers from the Input System

        //Physics sim steps forward, preceeded and suceeded by the scene's pre
        // and post physics updates, where prephysics handles the character game
        // logic like grounded and stuff, while post physics handles animation setting

        //Camera snapping to the respective target,

        //Game logic update, and sending animations settings to render system 

        //Render system renders it all 

        //Imgui system renders the game and editor GUI

        //windows system handles buffer swapping and polling of events
        //fancy way to say it takes stuff renderer drew (back buffer) and gives it to the 
        // monitor (front buffer) and then handles inputs(polling)
        // on thing to note is that the input polling is at the last, while input handling is first
        // this insn't wrong or right, because the run is a loop, and the variables and objects are  
        // preserved. basically, input -> phy and render -> poll is equivalent to poll-> input->  
        // render and phy

        // editing utils like escape cursor using F1
    }
}
```

### The Renderer

The rendering system is by far the largest subsystem in the engine, and also the one that gets rewritten the most. started with 2d rendering (just a triangle at that point), then to projection using glm, later model importer, shadows, Cook-Torrance PBR (using a bunch of physics formulae), bloom, and multi lighting all supported by imgui.

Unlike the rest of the engine, the renderer is split into several rendering passes instead of simply drawing everything directly to the screen. Each pass produces information that the following pass depends on, eventually resulting in the final image presented to the monitor. In effect, the same scene is currently rendered once for shadow mapping, then once for base rendering, and followed by multiple pingpong bloom iterations

The rendering pipeline currently looks something like this:

1. HDRI and model loading
2. IBL Precomputation
   (Environment Cubemap,
   Irradiance,
   Prefilter,
   BRDF LUT)
3. Shadow Pass
4. Main PBR Render Pass
5. Gamma Correction
6. Bright Pixel Extraction
7. Gaussian Bloom Blur
8. HDR Composite
9. Tone Mapping
10. Back Buffer

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
    Everything after Shadow pass is done every frame

#### How it Works

(skip to next section if you are famillar with how opengl rendering works)

There are 3 pipelines that feed the main render pipeline one after the other, the Assimp model processing pipeline, the OpenGL Rendering Pipeline, and the Renderer pipeling that i buit on top of it and the flow is as follows

1. Model File from blender or other app
2. ASSIMP Import Pipeline
3. CPU mesh objects
4. Opengl Upload pipeline, with the models as vertex buffer objects, and how to read vbo as vertex array objects, as well as later on, frame by frame transformation to both models and individual vertex based on animation and physics
5. GPU buffers
6. Vertex Shader per vertex
7. Rasterizer (to convert the vertex triangles to fragmets )
8. Fragment Shader per fragmets
9. Final Image

#### ASSIMP Pipeline

Handles asset importing and resides in model_loader

It takes in any common 3d file format, GLB(inbuilt binary textures while rest have separate texture folders), GLTF, FBX, OBJ.

Goes down the scene, adds a model, in the model adds a mesh, and in that mesh, iterating the file, adds vertices

End result is you get all this during initilization itself:

Scene owns models, Models own collision shapes, meshes and bonemaps, meshes own a vector list of  PBR textures, a list of vertices, and later on, has somewhat of an ownership of the buffer and array objects. The vertex owns a lot of data, from positions and texture coordinates, to bone weights for animation.

model loader's load model
```
void Model::loadModel(const std::string &path) {
importer = std::make_unique[Assimp::Importer](Assimp::Importer)();
unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
const aiScene *scene = importer->ReadFile(path, flags);
if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
std::cerr << "ERROR::ASSIMP::" << importer->GetErrorString() << std::endl;
return;
}
scene_ptr = scene;

if (scene->HasAnimations()) {
    std::cout << "Animations: "
              << scene->mNumAnimations
              << std::endl;
}
directory = path.substr(0, path.find_last_of('/'));
processNode(scene->mRootNode, scene, glm::mat4(1.0f));
computeBounds();
}
```

process nodes FIFO node loading for individual objects
```
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

Final process mesh to process vertices one at a time
```
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
std::vector<vertex> vertices;
std::vector<unsigned int> indices;
std::vector<texture> textures;

//vertex loop
for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    vertex v;
    // process the location and tex coords
    vertices.push_back(v);
}

//tangent, bitangent and bone system processing for animation

//texture map imports (currenlty called one at a time, function or loop implementation pending)

//finally processing and setting of vertex and textures and maps

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

All this is in ram as cpp objects, accessable by CPU but not quite by GPU yet.

#### OpenGL Pipeline

Responsible for transferring the model into GPU memory and eventually converting it into pixels.

This is where VAOs, VBOs, shaders and draw calls come into play. Most of it are standard calls present in any engine and are part of opengl. The rest, the shaders, the designing of what this pipelines does first, next, and carries over what, is the renderer pipeline.

I wont go into depth about how opengl does what it does, but look into renderpass for all the frame by frame setting of uniforms and the actual draw calls, modelloader's 3 draw functions for the draw logic for animation based on weights.

There, A VBO simply stores the raw vertex data inside GPU memory. Once uploaded, the mesh rarely needs to be sent again, allowing it to be rendered every frame with only a draw call.

The VAO doesn't store any vertex data itself. Instead, it remembers how the data inside one or more VBOs should be interpreted. For example, it tells OpenGL that the first three floats of every vertex represent its position, the next three represent the normal, followed by texture coordinates, tangents, bitangents, bone IDs and weights. EBO are also present, and work as a way to tell the gpu what all three vertices form a trangle

Once all this is done, the rendering is done as follows: the render binds the mesh's VAO, applies all transformations and the model, view and projection matrices (\< these three make it appear 3d), binds the texture adn issues the draw call glDrawElements. the opengl then executes the vertex shader for every vertex, assembles those vertices into triangles, rasterizes the triangles into fragments, and finally executes the fragment shader to determine the colour of every visible pixel.

one good example of this is the setup mesh from the modelloader

```
void Mesh::setupMesh() {

//The 3 objects:
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glGenBuffers(1, &EBO);
glBindVertexArray(VAO);

// binding of the data
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertex), &vertices[0], GL_STATIC_DRAW);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

//telling it how to read and pass the data to the shaders
// each one of these is passed from the VBO to the vertex shader
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                      (void *)offsetof(vertex, normal));
glEnableVertexAttribArray(2);
glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                      (void *)offsetof(vertex, texCoords));
glEnableVertexAttribArray(3);
glVertexAttribIPointer(3, 4, GL_INT, sizeof(vertex),
                       (void *)offsetof(vertex, boneIDs));
glEnableVertexAttribArray(4);
glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),
                      (void *)offsetof(vertex, weights));
glEnableVertexAttribArray(5);
glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                      (void *)offsetof(vertex, tangent));
glEnableVertexAttribArray(6);
glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                      (void *)offsetof(vertex, bitangent));
glBindVertexArray(0);
}
```
^^^ during initilization,
followed by multiple
```modelShader-> set<type>(<>,...);```
calls every frame and a draw call

draw system is placed rather neatly together in the modelLoader from line 209 and 261, and is overloaded to handel animations, colliders, ghosts, and general draw time transfrom skipping

#### PBR Renderer

The main renderer uses a Cook-Torrance BRDF with the GGX microfacet model.
After recieving stuff as the objects, the control is passed on to the shaders.
first we have a minimal vertex buffer controling animations, then to the fragments shader with actual Physically based rendering implementation.

PBR lives almost completely in the shaders, after setting of the uniforms and layout that is.
PBR is a method of rendering that enable us to give objects a metallic, roughness, AO, and emissive traits and make them look real.

The Model Loader extractes in MAP or Constant form
Base Color
Metallic
Roughness
Ambient Occlusion
Emissive

Each fragment computes
Normal Distribution Function (GGX)
Smith Geometry Function
Schlick Fresnel approximation

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

Some inspirations for the project and engine architecture were taken from the following sources:

- https://learnopengl.com/ (for OpenGL rendering and shader management)
- ogldev.com (for OpenGL rendering and shader management)
- lowlevelgamedev (moral support)

LOST HOURS LOG (not visible in any file anymore but still necessary for my journey and learning process)
--------------------------------------------------------------------------------------------------------

Hr 0 onwards, build started  (note render engine camera window transform, scene and entity management, physics, input, and time management were all in this at this point)

1. Hr 3 first triangle rendered
2. Hr 6 first 3d cube
3. Hr 10 first 3d model loaded and rendered
4. Hr 12 first 3d model with texture
5. Hr 15 tried instancing, was not really neaded partially removed
6. Hr 18 implemented height map import (still has an exe on github) but not really needed removed, created terrain from actual himalyan ranges and coloured it procedurally accourding to slope, amazing stuff imo
7. Hr 18 going to refactor the engine into a more modular architecture with systems and utils, and a core engine class to manage them all. ps first of 3 refactors
8. Hr 21 onwards the engine started to resemble this one, with the creating a new model loader to load models, rendersystem for all rendering and a base engine class keeping it all separate
Hr 21-31+
