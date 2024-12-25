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
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iomanip>

// Files import
#include <render/shader.h>
#include "helpers.h"

// Objects include
#include "objects/skybox/skybox.h"
#include "objects/cube/cube.h"

#include "objects/obj/gltfObj.h"

//---- Scaling to make things more simple to follow for me ----

// Worldscale - A unit is approximately 10.0f
static float worldScale = 10.0f;

// Boundaries
static float domeScale      = 15.0f * worldScale;
static float domeBoundIn    = 11.9f * worldScale;
static float domeBoundOut   = 20.0f * worldScale;

// Ships bound
float boundary = 1500*3;

//---- Managing movement consequences ----

// Movement variables
glm::mat4 moveRotationMat;
GLfloat moveDist    = 0.1f * worldScale;
GLfloat moveAngle   = glm::radians(3.0f);
GLfloat tolerance   = 10.0f;

// Handle the case where the viewer is in the dome
bool inDome = true;

// Handle the case where the viewer is in space
bool inSpace = false;

// Skybox scale modifier
float skyboxSclMod = 1.0f;
float domeSclMod = 1.0f;

//---- Base openGL things ----

// Static elements
static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

// Camera
static glm::vec3 eye_center (0.0f, 20.f, 0.0f);
static glm::vec3 lookat     (0.0f, 0.0f, 100.0f);
static glm::vec3 up         (0.0f, 1.0f, 0.0f);
static float FoV    = 60.0f;
static float zNear  = 10.0f;
static float zFar   = 3*boundary;

//---- Shaders ----

// Shaders dictionary
std::map<std::string,GLuint> shaders;

//---- Shadows ----

// Lighting
static glm::vec3 lightIntensity(0.25e6f);

// Frame buffer stuff
GLuint depthFBO;
GLuint depthTexture;

// Shadow mapping
static int depthMapWidth = 1024;
static int depthMapHeight = 758;

// Depth camera settings
static glm::vec3 lightPosition  (0.0, 1.1f * domeScale, 0.0f);
static glm::vec3 depthlookat    (0.0f, 0.0f, 0.0f);
static glm::vec3 lightUp        (1, 0, 0);
static float depthFoV   = 110.0f;
static float depthNear  = 50.0f;
static float depthFar   = 400.0f;

//---- Animation ----

// Animation
static bool playAnimation = true;
static float playbackSpeed = 1.5f;

// FPS tracking
float deltaTime;
static double lastTime = glfwGetTime();
float thetime = 0.0f;			            // Animation time
float fTime = 0.0f;			                // Time for measuring fps
unsigned long frames = 0;

//---- Methods ----

std::map<std::string,GLuint> LoadShaders()
{
    std::map<std::string,GLuint> shaderlist;

    GLuint programID = LoadShadersFromFile("../src/shaders/obj/obj_def.vert", "../src/shaders/obj/obj_def.frag");
    GLuint depthProgramID = LoadShadersFromFile("../src/shaders/obj/obj_dpth.vert", "../src/shaders/obj/obj_dpth.frag");
    GLuint shadowProgramID = LoadShadersFromFile("../src/shaders/obj/obj_s.vert", "../src/shaders/obj/obj_s.frag");
    GLuint instancedshadowProgramID = LoadShadersFromFile("../src/shaders/obj/obj_si.vert", "../src/shaders/obj/obj_s.frag");
    GLuint depthProgramID_i = LoadShadersFromFile("../src/shaders/obj/obj_dpth_i.vert", "../src/shaders/obj/obj_dpth.frag");

    if (programID == 0 || depthProgramID == 0 || shadowProgramID == 0 || instancedshadowProgramID == 0 || depthProgramID_i == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }
    shaderlist["obj_def"] = programID;
    shaderlist["obj_dpth"] = depthProgramID;
    shaderlist["obj_s"] = shadowProgramID;
    shaderlist["obj_si"] = instancedshadowProgramID;
    shaderlist["obj_dpth_i"] = depthProgramID_i;

    return shaderlist;
}

//---
// Midpoint algorithm viewed from : https://www.youtube.com/watch?v=hpiILbMkF9w
// Changed position to use a positive r and to fill the circle

std::vector<glm::vec3> calcDomeGrid(int pixelSize, int cx, int cy)
{
    std::vector<glm::vec3> pos_vector;

    // Initialise radius according to pixel side (how many pixels there is)
    int r = domeBoundIn / pixelSize;

    // Init the local circle positions
    int x = 0;
    int z = r;
    int p = 1-r;

    // Main algorithm loop
    while (x <= z)
    {
        // Add "pixel" position to the vector
        for (int i = cx - x * pixelSize; i <= cx + x * pixelSize; i += pixelSize) {
            pos_vector.push_back(glm::vec3(i, 0, cy + z * pixelSize)); // Top part
            pos_vector.push_back(glm::vec3(i, 0, cy - z * pixelSize)); // Bottom part
        }
        for (int i = cx - z * pixelSize; i <= cx + z * pixelSize; i += pixelSize) {
            pos_vector.push_back(glm::vec3(i, 0, cy + x * pixelSize)); // Right part
            pos_vector.push_back(glm::vec3(i, 0, cy - x * pixelSize)); // Left part
        }

        // Midpoint outside circle
        if (p < 0){
            p += 2 * x + 3;
        }
        // Midpoint inside circle
        else{
            p += 2 * (x - z) + 5;
            z--;
        }
        // Got to next point
        x++;
    }

    return pos_vector;
};

//---

void prepShips(std::map<std::string,GLuint> shaders, gltfObj ships[6],int blockBindFloor)
{
    std::string names[3] = {"virgo","scorpio","gemini"};
    float scales[3] = {2*8.0f,2*6.0f,2*5.0f};
    float rot[3] = {-90.0f,-90.0f,-90.0f};

    for (int i = 0; i < 3; ++i)
    {
        std::string modelPath = "../assets/models/ships/" + names[i] + ".gltf";
        std::string texturePath = "../assets/textures/ships/" + names[i] + ".png";

        ships[i].init_s();
        ships[i].init_plmt(glm::vec3(-2*boundary,0.0f,0.0f),glm::vec3(worldScale*scales[i]),glm::vec3(0.0f,1.0f,0.0f),rot[i]);
        ships[i].init(shaders["obj_def"],shaders["obj_dpth"],i + blockBindFloor,modelPath.c_str(), texturePath.c_str());
    }
    for (int i = 3; i < 6; ++i)
    {
        std::string modelPath = "../assets/models/ships/" + names[i-3] + ".gltf";
        std::string texturePath = "../assets/textures/ships/" + names[i-3] + ".png";

        ships[i].init_s();
        ships[i].init_plmt(glm::vec3(-2*boundary,0.0f,0.0f),glm::vec3(worldScale*scales[i-3]),glm::vec3(0.0f,1.0f,0.0f),rot[i-3]);
        ships[i].init(shaders["obj_def"],shaders["obj_dpth"],i + blockBindFloor,modelPath.c_str(), texturePath.c_str());
    }
}

//---- Managing ship movement ----

//---- Variables -----

bool virgoOOB,scorpioOOB,geminiOOB,virgo1OOB,scorpio1OOB,gemini1OOB;
bool shipsOOB[6] = {virgoOOB,scorpioOOB,geminiOOB,virgo1OOB,scorpio1OOB,gemini1OOB};


//---- Methods ----

void isOOB(gltfObj ships[6], float bound)
{
    for(int i = 0; i < 6; i++)
    {
        if(ships[i].position.x < -bound){
            shipsOOB[i] = true;
        }
    }
}

glm::vec3 genShipxzy(float bound){

    int zBound = bound/2;
    int yBound = bound/2;
    int xBound = bound;

    // Generate the new positions
    int y = 0 + rand() % yBound;
    int z = -zBound + rand() % 2*zBound;
    int x = bound + rand() % xBound;

    if (y <= 500)
    {
        if(y % 2 == 0){
            z = -zBound + rand() % (zBound-500);
        }else{
            z = 500 + rand() % (zBound-500);
        }
    }
    // Send the result as vector
    return glm::vec3(x,y,z);

}

void moveShips(gltfObj ships[6], float currentStep)
{
    for(int i = 0; i < 6; i++)
    {
        // Update the ships boolean
        isOOB(ships,boundary);

        // Get the entry point for ships OOB
        if (shipsOOB[i]){
            ships[i].position = genShipxzy(boundary);
            shipsOOB[i] = false;
        }

        // Move the ships (along the x axis)
        ships[i].init_plmt(
           ships[i].position + glm::vec3(-currentStep,0.0f,0.0f),
           ships[i].scale,
           ships[i].rotationAxis,
           ships[i].rotationAngle);
    }
};


//---- Back to unrelated methods ----

static void saveDepthTexture(GLuint fbo, std::string filename) {
    int width = depthMapWidth;
    int height = depthMapHeight;
    if (depthMapWidth == 0 || depthMapHeight == 0) {
        width = windowWidth;
        height = windowHeight;
    }
    int channels = 3;

    std::vector<float> depth(width * height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_DEPTH_COMPONENT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;
    std::ofstream file_out("depthData.txt");
    for (int i = 0; i < width * height; ++i)
    {
        file_out << depth[i] << " ";
        if (i % width == width - 1) file_out << "\n";
    }

    stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}

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

        glm::vec3 mvtFB;
        glm::vec3 mvtLR;

        if (inDome)
        {
            mvtFB = moveDist * dirVect;
            mvtLR = moveDist * dirSide;
        }
        else if (inSpace)
        {
            float d = glm::length(eye_center);
            float dim = glm::min(1.0f,(boundary*0.6f-d) / d);

            std::cout << dim << std::endl;

            skyboxSclMod = 1.0f + (1-dim)*0.75;
            domeSclMod = dim;

            mvtFB = (moveDist*dim) * dirVect;
            mvtLR = (moveDist*dim) * dirSide;
        }


        // Forward
        if (key == GLFW_KEY_Z || key == GLFW_KEY_W)
        {
            if (glm::length(eye_center + mvtFB) < domeBoundIn + tolerance)
            {
                eye_center += mvtFB;
                lookat += mvtFB;
            }
            else if (glm::length(eye_center + mvtFB) > domeBoundOut - tolerance)
            {
                eye_center += mvtFB;
                lookat += mvtFB;
            }
        }
        // Backward
        if (key == GLFW_KEY_S)
        {
            if (glm::length(eye_center - mvtFB) < domeBoundIn + tolerance)
            {
                eye_center -= mvtFB;
                lookat -= mvtFB;

            }else if (glm::length(eye_center - mvtFB) > domeBoundOut - tolerance)
            {
                eye_center -= mvtFB;
                lookat -= mvtFB;
            }
        }
        // Left
        if (key == GLFW_KEY_Q || key == GLFW_KEY_A)
        {
            if (glm::length(eye_center + mvtLR) < domeBoundIn + tolerance)
            {
                eye_center += mvtLR;
                lookat += mvtLR;

            }else if (glm::length(eye_center + mvtLR) > domeBoundOut - tolerance)
            {
                eye_center += mvtLR;
                lookat += mvtLR;

            }
        }
        // Right
        if (key == GLFW_KEY_D)
        {
            if (glm::length(eye_center - mvtLR) < domeBoundIn + tolerance)
            {
                eye_center -= mvtLR;
                lookat -= mvtLR;

            }else if (glm::length(eye_center - mvtLR) > domeBoundOut - tolerance)
            {
                eye_center -= mvtLR;
                lookat -= mvtLR;

            }
        }

        // Get in or out of the dome
        if (key == GLFW_KEY_E && glm::length(eye_center - glm::vec3(150.0f,20.0f,0.0f)) < 70.0f)
        {
            if (inDome)
            {
                inSpace = true;

                // Put the viewer in the dome
                eye_center = glm::vec3(domeBoundOut + 20.0f,20.0f,0.0f);
                lookat = glm::vec3(domeBoundOut + 20.0f + 100.0f,20.0f,0.0f);

                inDome = false;
            }
            else{
                if (inSpace)
                {
                    inDome = true;

                    // Put the viewer in the dome
                    eye_center = glm::vec3(domeBoundIn - 20.0f,20.0f,0.0f);
                    lookat = glm::vec3(domeBoundIn - 20.0f - 100.0f,20.0f,0.0f);

                    inSpace = false;
                }
            }
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

#endif //MAIN_H
