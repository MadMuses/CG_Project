#ifndef MAIN_H
#define MAIN_H

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
#include "objects/skybox/skybox.h"
#include "objects/cube/cube.h"

#include "objects/obj/staticObj.h"
#include "objects/obj/movingObj.h"

// Static elements
static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

// Worldscale - A meter is approximately 10.0f
static float worldScale = 10.0f;
static float domeScale = 15.0f;

// Camera
static glm::vec3 eye_center(0.0f, 20.0f, 0.0f);
static glm::vec3 lookat(100.0f, 0.0f, 0.0f);
static glm::vec3 up(0.0f, 1.0f, 0.0f);
static float FoV = 45.0f;
static float zNear = 10.0f;
static float zFar = 3000.0f;

// Lighting
static glm::vec3 lightIntensity(1e6f);
static glm::vec3 lightPosition(0.0, 1.1f * worldScale * domeScale, 0.0f);

// Animation
static bool playAnimation = true;
static float playbackSpeed = 2.0f;

// FPS tracking
float deltaTime;
static double lastTime = glfwGetTime();
float thetime = 0.0f;			            // Animation time
float fTime = 0.0f;			                // Time for measuring fps
unsigned long frames = 0;

// Shaders dictionary
std::map<std::string,GLuint> shaders;

// Movement variables
GLfloat moveDist = 0.1f*worldScale;
GLfloat moveAngle = glm::radians(3.0f);
glm::mat4 moveRotationMat;

// Boundaries
float domeBoundIn = 0.9f*worldScale*domeScale;
float domeBoundOut = 1.1f*worldScale*domeScale;

// Some helpers needing the global variables
void calcframerate()
{
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
};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (action == GLFW_REPEAT || action == GLFW_PRESS)
    {
        glm::vec3 v = normalize(lookat-eye_center);
        glm::vec3 dirVect = normalize(glm::vec3(v.x,0,v.z));
        glm::vec3 dirSide = normalize(glm::vec3(cross(up,v).x,0,cross(up,v).z));

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
        if (key == GLFW_KEY_H)
        {
            lightPosition += glm::vec3(0,5,0);
            printVec(lightPosition);
        }
        if (key == GLFW_KEY_J)
        {
            lightPosition -= glm::vec3(0,5,0);
            printVec(lightPosition);
        }
    }
};

#endif //MAIN_H
