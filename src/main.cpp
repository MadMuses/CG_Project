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

    // Our 3D character
    Skybox skybox;
    Cube lightcube;

    staticObj dome;
    staticObj virgo;

    movingObj bot;


    // initializing objects
    lightcube.initialize(lightPosition);
    skybox.initialize(glm::vec3(worldScale*100));

    dome.initialize(shaders["objBasic"],0,"../assets/models/dome/dome.gltf", NULL,
        glm::vec3(0.0f),
        glm::vec3(domeScale * worldScale),
        glm::vec3(0.0f,1.0f,0.0f),
        0.0f);

    virgo.initialize(shaders["objBasic"],2,"../assets/models/ships/virgo.gltf", "../assets/textures/ships/virgo.png",
        glm::vec3(200.0f,0.0f,200.0f),
        glm::vec3(worldScale*5));

    bot.initialize(shaders["objBasic"],1,"../assets/models/bot/bot.gltf");

    // Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

    do
    {
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
        bot.render(vp,lightPosition,lightIntensity);
        dome.render(vp,lightPosition,lightIntensity);

        virgo.render(vp,lightPosition,lightIntensity);

        // Count number of frames over a few seconds and take average
        calcframerate();

        if (playAnimation) {
            thetime += deltaTime * playbackSpeed;
            bot.update(thetime);
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

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
};