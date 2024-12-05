#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// GLTF model loader
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>



#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iomanip>

// Files import
#include <render/shader.h>
#include "helpers.h"

// Objects include
#include "objects/staticObj.h"

// Static elements
static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

// Call back functions
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Camera
static glm::vec3 eye_center(80.0f, 0.0f, 0.0f);
static glm::vec3 lookat(0.0f, 0.0f, 0.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 10.0f;
static float zFar = 1500.0f;

// Lighting
static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(0.0, 0.0f, 0.0f);

// Animation
static bool playAnimation = true;
static float playbackSpeed = 2.0f;

int main(void)
{
    // Initalise window and OpenGl functions
    window = initOpenGL(windowWidth, windowHeight);
    if (window == NULL){return -1;};

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetKeyCallback(window, key_callback);

    // Background
    glClearColor(0.2f, 0.2f, 0.2f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    GLuint programID = LoadShadersFromFile("../src/shaders/bot.vert", "../src/shaders/bot.frag");
    if (programID == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }

    // Our 3D character
    staticObj dome;
    dome.initialize(programID,0,"../src/models/dome/dome.gltf", glm::vec3(0.0f),glm::vec3(500.0f));

    staticObj bot;
    bot.initialize(programID,1,"../src/models/bot/bot.gltf");

    // Time and frame rate tracking
    static double lastTime = glfwGetTime();
    float time = 0.0f;			// Animation time
    float fTime = 0.0f;			// Time for measuring fps
    unsigned long frames = 0;

    // Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update states for animation
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        // Rendering
        viewMatrix = glm::lookAt(eye_center, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;

        bot.render(vp,lightPosition,lightIntensity);
        dome.render(vp,lightPosition,lightIntensity);

        // FPS tracking
        // Count number of frames over a few seconds and take average
        frames++;
        fTime += deltaTime;
        if (fTime > 2.0f) {
            float fps = frames / fTime;
            frames = 0;
            fTime = 0;

            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << "Final Project | Frames per second (FPS): " << fps;
            glfwSetWindowTitle(window, stream.str().c_str());
        }

        if (playAnimation) {
            time += deltaTime * playbackSpeed;
        }

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (!glfwWindowShouldClose(window));

    // Clean up

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

// Movement variables
GLfloat moveDist = 1.0f;
GLfloat moveAngle = glm::radians(3.0f);
glm::vec3 v;
glm::mat4 moveRotationMat;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_REPEAT || action == GLFW_PRESS)
    {
        v = normalize(lookat-eye_center);
        glm::vec3 dirVect = normalize(glm::vec3(v.x,0,v.z));
        glm::vec3 dirSide = normalize(glm::vec3(cross(up,v).x,0,cross(up,v).z));
        // Person movement

        // Forward
        if (key == GLFW_KEY_Z || key == GLFW_KEY_W)
        {
            eye_center += moveDist * dirVect;
            lookat += moveDist * dirVect;
        }
        // Backward
        if (key == GLFW_KEY_S)
        {
            eye_center -= moveDist * dirVect;
            lookat -= moveDist * dirVect;
        }
        // Left
        if (key == GLFW_KEY_Q || key == GLFW_KEY_A)
        {
            eye_center += moveDist * dirSide;
            lookat += moveDist * dirSide;
        }
        // Right
        if (key == GLFW_KEY_D)
        {
            eye_center -= moveDist * dirSide;
            lookat -= moveDist * dirSide;
        }

        // Handling view movement
        if (key == GLFW_KEY_UP)
        {
            if (v.y < 0.999) // Safeguard to prevent visual bugs
            {
                moveRotationMat = glm::rotate(glm::mat4(1.0f), moveAngle*0.75f, cross(v,up));
                lookat = eye_center + glm::vec3(moveRotationMat*glm::vec4(v,1.0f));
            }
        }

        if (key == GLFW_KEY_DOWN)
        {
            if (v.y > -0.996) // Safeguard to prevent visual bugs
            {
                moveRotationMat = glm::rotate(glm::mat4(1.0f), moveAngle*0.75f, cross(up,v));
                lookat = eye_center + glm::vec3(moveRotationMat*glm::vec4(v,1.0f));
            }
        }

        if (key == GLFW_KEY_LEFT)
        {
            moveRotationMat = glm::rotate(glm::mat4(1.0f), moveAngle, up);
            lookat = eye_center + glm::vec3(moveRotationMat*glm::vec4(v,1.0f));

        }

        if (key == GLFW_KEY_RIGHT)
        {
            moveRotationMat = glm::rotate(glm::mat4(1.0f), moveAngle, -up);
            lookat = eye_center + glm::vec3(moveRotationMat*glm::vec4(v,1.0f));
        }
    }
};