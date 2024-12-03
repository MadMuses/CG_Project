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
#include "helpers.cpp"

// Objects include

// Static elements
static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

// Camera
static glm::vec3 eye_center(0.0f, 100.0f, 800.0f);
static glm::vec3 lookat(0.0f, 0.0f, 0.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 100.0f;
static float zFar = 1500.0f;

// Lighting
static glm::vec3 lightIntensity(5e6f, 5e6f, 5e6f);
static glm::vec3 lightPosition(-275.0f, 500.0f, 800.0f);

int main(void)
{
    // Initalise window and OpenGl functions
    window = initOpenGL(windowWidth, windowHeight);
    if (window == NULL){return -1;};

    // Background
    glClearColor(0.2f, 0.2f, 0.2f, 0.f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Camera setup
    glm::mat4 viewMatrix, projectionMatrix;
    projectionMatrix = glm::perspective(glm::radians(FoV), (float)windowWidth / windowHeight, zNear, zFar);

    do
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Camera view point
        viewMatrix = glm::lookAt(eye_center, lookat, up);
        glm::mat4 vp = projectionMatrix * viewMatrix;

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
