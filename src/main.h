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
//#include "objects/obj/movingObj.h"
#include "objects/obj/gltfObj.h"

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
static glm::vec3 lightIntensity(0.25e6f);

// Shadow mapping
static int depthMapWidth = 1024;
static int depthMapHeight = 758;

// Depth camera settings
static glm::vec3 lightPosition(0.0, 1.1f * worldScale * domeScale, 0.0f);
static glm::vec3 lightUp(0, 0, 1);
static glm::vec3 depthlookat(0.0f, 0.0f, 0.0f);
static float depthFoV = 90.0f;
static float depthNear = 50.0f;
static float depthFar = 400.0f;

// Frame buffer stuff
GLuint depthFBO;
GLuint depthTexture;

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

void prepShips(std::map<std::string,GLuint> shaders, staticObj ships[3],float worldScale)
{
    std::string names[3] = {"virgo","scorpio","gemini"};
    float scales[3] = {8.0f,6.0f,5.0f};

    for (int i = 0; i < 3; ++i)
    {
        std::string modelPath = "../assets/models/ships/" + names[i] + ".gltf";
        std::string texturePath = "../assets/textures/ships/" + names[i] + ".png";

        ships[i].init_s(shaders["obj_dpth"]);
        ships[i].initialize(shaders["obj_def"],i,modelPath.c_str(), texturePath.c_str(),
        glm::vec3(300.0f,0.0f,-300.0f + 300.0f*i),
        glm::vec3(worldScale*scales[i]));
    }
}

void prepNature(std::map<std::string,GLuint> shaders,staticObj plants[7],float worldScale, int blockBindFloor)
{
    std::string names[4] = {"grass_2","grass_3","grass_4.1","grass_4.2"};
    std::string treenames[2] = {"spruce","oak"};
    glm::vec3 treepositions[2] = {glm::vec3(20.0f,0.0f,80.0f),glm::vec3(60.0f,0.0f,-40.0f)};

    for (int i = 0; i < 4; ++i)
    {
        std::string modelPath = "../assets/models/nature/" + names[i] + ".gltf";
        plants[i].init_s(shaders["obj_dpth"]);
        plants[i].initialize(shaders["obj_s"],i + blockBindFloor,modelPath.c_str(), NULL,
        glm::vec3(40.0f,0.0f,-60.0f + 30.0f*i),
        glm::vec3(worldScale*0.5));
    }

    plants[4].init_s(shaders["obj_dpth"]);
    plants[4].initialize(shaders["obj_s"],4+blockBindFloor,"../assets/models/nature/flower.gltf", "../assets/textures/nature/flowers.png",
    glm::vec3(50.0f,0.0f,0.0f),
    glm::vec3(worldScale*0.5));

    for (int j = 5; j < 7; ++j)
    {
        std::string treemodelPath = "../assets/models/nature/" + treenames[j-5] + ".gltf";

        plants[j].init_s(shaders["obj_dpth"]);
        plants[j].initialize(shaders["obj_s"],j + blockBindFloor,treemodelPath.c_str(), "../assets/textures/nature/trees.png",
        treepositions[j-5],
        glm::vec3(worldScale*2.5));
    }

}

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

std::map<std::string,GLuint> LoadShaders()
{
    std::map<std::string,GLuint> shaderlist;

    GLuint programID = LoadShadersFromFile("../src/shaders/obj_def.vert", "../src/shaders/obj_def.frag");
    GLuint depthProgramID = LoadShadersFromFile("../src/shaders/obj_dpth.vert", "../src/shaders/obj_dpth.frag");
    GLuint shadowProgramID = LoadShadersFromFile("../src/shaders/obj_s.vert", "../src/shaders/obj_s.frag");
    GLuint instancedshadowProgramID = LoadShadersFromFile("../src/shaders/obj_si.vert", "../src/shaders/obj_s.frag");
    GLuint depthProgramID_i = LoadShadersFromFile("../src/shaders/obj_dpth_i.vert", "../src/shaders/obj_dpth.frag");

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
