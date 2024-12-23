#include "main.h"

bool saveDepth = true;

int main(void)
{
    // Initalise window and OpenGl functions
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(windowWidth, windowHeight, "Final project", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to open a GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Load OpenGL functions, gladLoadGL returns the loaded version, 0 on error.
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        std::cerr << "Failed to initialize OpenGL context." << std::endl;
        return -1;
    }

    // Prepare shadow map size for shadow mapping. Usually this is the size of the window itself, but on some platforms like Mac this can be 2x the size of the window. Use glfwGetFramebufferSize to get the shadow map size properly.
    glfwGetFramebufferSize(window, &depthMapWidth, &depthMapHeight);

    // Background
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Create a frame buffer
    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

    // Create Framebuffer Texture
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depthMapWidth, depthMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Framebuffer is not complete." << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Load shaders
    shaders = LoadShaders();

    // Generate values from instancing :
    genInstancingVal();

// Create all of the objects :

    // Basic objects
    Skybox skybox;
    Cube lightcube;

    // Render ships
    gltfObj virgo,gemini,scorpio;
    gltfObj ships[3] = {virgo,gemini,scorpio};
    prepShips(shaders,ships,7);

    // Most complicated to setup (instancing) : Plants
    gltfObj grass2,grass3,grass41,grass42;
    gltfObj grass[4] = {grass2,grass3,grass41,grass42};

    // Placing the flowers
    gltfObj flowers;
    gltfObj flowers2;

    // Static Obj : Trees
    gltfObj spruce,oak;

    // Static object : The dome
    gltfObj dome;

// Initialise objects :

    // initializing objects
    lightcube.initialize(lightPosition);
    skybox.initialize(glm::vec3(worldScale*100));

    // Static Obj : Grass blocks
    // Grass elements
    std::string names[4] = {"grass_2","grass_3","grass_4.1","grass_4.2"};

    // Position + shadow inits
    for (int i = 0; i < 4; ++i)
    {
        grass[i].init_s();
        grass[i].init_plmt(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.5f * worldScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    }

    // Instancing position buffer changes so we init them here
    grass[0].init_i(grassAmount,grass_pos_2,grass_scl,grass_angl);
    grass[1].init_i(grassAmount,grass_pos_3,grass_scl,grass_angl);
    grass[2].init_i(grassAmount,grass_pos_41,grass_scl,grass_angl);
    grass[3].init_i(grassAmount,grass_pos_42,grass_scl,grass_angl);

    // Global init
    for (int i = 0; i < 4; ++i)
    {
        std::string modelPath = "../assets/models/nature/" + names[i] + ".gltf";
        grass[i].init(shaders["obj_si"],shaders["obj_dpth_i"],i,modelPath.c_str(), NULL);
    }

    oak.init_s();
    oak.init_plmt(glm::vec3(0.0f),glm::vec3(worldScale*2.5),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    oak.init_i(3,oak_pos,oak_scl,oak_angl);
    oak.init(shaders["obj_si"],shaders["obj_dpth_i"],5,"../assets/models/nature/oak.gltf", "../assets/textures/nature/trees.png");

    spruce.init_s();
    spruce.init_plmt(glm::vec3(0.0f),glm::vec3(worldScale*2.5),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    spruce.init_i(2,spruce_pos,spruce_scl,spruce_angl);
    spruce.init(shaders["obj_si"],shaders["obj_dpth_i"],6,"../assets/models/nature/spruce.gltf", "../assets/textures/nature/trees.png");

    flowers.init_plmt(glm::vec3(-7.0f * 7.0f, 0.0f, -9.0f * 7.0f),glm::vec3(0.5f * worldScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    flowers.init_s();
    flowers.init_i(flowerAmount,flowers_pos,flowers_scl,flowers_angl);
    flowers.init(shaders["obj_si"],shaders["obj_dpth_i"],20,"../assets/models/nature/flower.gltf", "../assets/textures/nature/flowers.png");

    flowers2.init_plmt(glm::vec3(2.0f * 7.0f, 0.0f, 9.0f*7.0f),glm::vec3(0.5f * worldScale),glm::vec3(0.0f,1.0f,0.0f),0.0f);
    flowers2.init_s();
    flowers2.init_i(flowerAmount,flowers_pos,flowers_scl,flowers_angl);
    flowers2.init(shaders["obj_si"],shaders["obj_dpth_i"],20,"../assets/models/nature/flower.gltf", "../assets/textures/nature/flowers.png");

    dome.init_s();
    dome.init_plmt(glm::vec3(0.0f),glm::vec3(domeScale),glm::vec3(0.0f,1.0f,0.0f),42.5f);
    dome.init(shaders["obj_s"],shaders["obj_dpth"],10,"../assets/models/dome/dome.gltf", NULL);

// The two different cameras

    // Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

    // Light POV Camera setup
    glm::mat4 lightViewMatrix, lightProjectionMatrix;
    lightProjectionMatrix = glm::perspective(glm::radians(depthFoV), (float)depthMapWidth / depthMapHeight, depthNear, depthFar);

// "Game" loop
    do
    {
    // Managing the depth texture creation
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        lightViewMatrix = glm::lookAt(lightPosition,depthlookat,lightUp);
        glm::mat4 lvp = lightProjectionMatrix * lightViewMatrix;

        dome.depthRender(lvp);

        flowers.depthRender(lvp);
        flowers2.depthRender(lvp);

        for (int i =0; i < 4; i++)
        {
            grass[i].depthRender(lvp);
        }

        oak.depthRender(lvp);
        spruce.depthRender(lvp);

        if (saveDepth) {
            std::string filename = "depth_camera.png";
            saveDepthTexture(depthFBO, filename);
            std::cout << "Depth texture saved to " << filename << std::endl;
            saveDepth = false;
        }

    // Rendering the scene
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update states for animation
        double currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        // Rendering
        viewMatrix = glm::lookAt(eye_center, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;

        skybox.render(vp);
        lightcube.render(vp,lightPosition);

        dome.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        flowers.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
        flowers2.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        for (int i =0; i < 4; i++){grass[i].render(vp,lightPosition,lightIntensity,lvp,depthTexture);}
        oak.render(vp,lightPosition,lightIntensity,lvp,depthTexture);
        spruce.render(vp,lightPosition,lightIntensity,lvp,depthTexture);

        // Count number of frames over a few seconds and take average
        calcframerate();

        if (playAnimation) {
            thetime += deltaTime * playbackSpeed;
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (!glfwWindowShouldClose(window));

    // Clean up
    skybox.cleanup();
    dome.cleanup();
    lightcube.cleanup();
    for (int i=0; i < 3;i++){ships[i].cleanup();}
    for (int i=0; i < 4;i++){grass[i].cleanup();}
    oak.cleanup();
    spruce.cleanup();
    flowers.cleanup();
    flowers2.cleanup();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
};