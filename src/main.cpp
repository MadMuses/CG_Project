#include "main.h"

int main(void)
{
    // Initalise window and OpenGl functions
    window = initOpenGL(windowWidth, windowHeight);
    if (window == NULL){return -1;};

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Background
    glClearColor(0.0f, 0.0f, 0.0f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Load shaders
    shaders = LoadShaders();

    // Create depth FBO and Texture
    LoadFBO(depthFBO,depthTexture,depthMapWidth,depthMapHeight);

    // All of our models
    Skybox skybox;
    Cube lightcube;

    staticObj dome;

    staticObj virgo,gemini,scorpio;
    staticObj grass2,grass3,grass41,grass42,flower,spruce,oak;

    staticObj ships[3] = {virgo,gemini,scorpio};
    prepShips(shaders,ships,worldScale);

    staticObj plants[7] = {grass2,grass3,grass41,grass42,flower,spruce,oak};
    prepNature(shaders,plants,worldScale,5);

    // initializing objects
    lightcube.initialize(lightPosition);
    skybox.initialize(glm::vec3(worldScale*100));

    dome.initialize(shaders["objBasic"],shaders["objDepth"],30,"../assets/models/dome/dome.gltf", NULL,
        glm::vec3(0.0f),
        glm::vec3(domeScale * worldScale),
        glm::vec3(0.0f,1.0f,0.0f),
        0.0f);

    // Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

    // Light POV Camera setup
    glm::mat4 lightViewMatrix, lightProjectionMatrix;
    lightProjectionMatrix = glm::perspective(glm::radians(depthFoV), (float)depthMapWidth / depthMapHeight, depthNear, depthFar);

    bool saveDepth = true;
    do
    {
    // Managing the depth texture creation
        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        lightViewMatrix = glm::lookAt(lightPosition,depthlookat,lightUp);
        glm::mat4 lvp = lightProjectionMatrix * lightViewMatrix;

        dome.depthRender(lvp);
        for (int i=0; i < 7;i++)
        {
            plants[i].depthRender(lvp);
        }

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

        dome.render(vp,lightPosition,lightIntensity);

        for (int i=0; i < 3;i++)
        {
            ships[i].render(vp,lightPosition,lightIntensity);
        }
        for (int i=0; i < 7;i++)
        {
            plants[i].render(vp,lightPosition,lightIntensity);
        }

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
    for (int i=0; i < 7;i++){plants[i].cleanup();}

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
};