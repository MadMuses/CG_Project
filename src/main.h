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
static float domeBoundOut   = 17.0f * worldScale;

// Movement variables
glm::mat4 moveRotationMat;
GLfloat moveDist    = 0.1f * worldScale;
GLfloat moveAngle   = glm::radians(3.0f);
GLfloat tolerance   = 10.0f;

//---- Base openGL things ----

// Static elements
static GLFWwindow *window;
static int windowWidth = 1024;
static int windowHeight = 768;

// Camera
static glm::vec3 eye_center (0.0f, 20.f, 0.0f);
static glm::vec3 lookat     (100.0f, 0.0f, 0.0f);
static glm::vec3 up         (0.0f, 1.0f, 0.0f);
static float FoV    = 45.0f;
static float zNear  = 10.0f;
static float zFar   = 3000.0f;

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
static glm::vec3 lightUp        (0, 0, 1);
static float depthFoV   = 90.0f;
static float depthNear  = 50.0f;
static float depthFar   = 400.0f;

//---- Animation ----

// Animation
static bool playAnimation = true;
static float playbackSpeed = 2.0f;

// FPS tracking
float deltaTime;
static double lastTime = glfwGetTime();
float thetime = 0.0f;			            // Animation time
float fTime = 0.0f;			                // Time for measuring fps
unsigned long frames = 0;

//---- Instancing values ----

// Grass
std::vector<glm::vec3> calcDomeGrid(int pixelSize, int cx, int cy);

// Creating the grid with all positions
std::vector<glm::vec3> pos_vector = calcDomeGrid(7,0,0);
int grassAmount = pos_vector.size()/4;

// General grid position
GLfloat grid_pos[pos_vector.size() * 3];

// Create the grass position attributes
GLfloat grass_pos_2[grassAmount*3];
GLfloat grass_pos_3[grassAmount*3];
GLfloat grass_pos_41[grassAmount*3];
GLfloat grass_pos_42[grassAmount*3];

// Create scale and rotation
GLfloat grass_scl[grassAmount];
GLfloat grass_angl[grassAmount];

// FLOWER PATCH LETS GOO
int flowerAmount = 52;
GLfloat flowers_pos[flowerAmount*3];
GLfloat flowers_scl[flowerAmount];
GLfloat flowers_angl[flowerAmount];

// Planting the trees
GLfloat oak_pos[9] = {
    -70.0f,0.0f,-28.0f,
    63.0f,0.0f,35.0f,
    35.0f,0.0f,-42.0f
};
GLfloat oak_scl[3] = {0.5f,1.1f,0.9f};
GLfloat oak_angl[3] = {0.0f,63.0f,45.0f};

GLfloat spruce_pos[6] = {
    -14.0f,0.0f,21.0f,
    -63.0f,0.0f,49.0f
};
GLfloat spruce_scl[2] = {0.9f,1.4f};
GLfloat spruce_angl[2] = {0.0f,63.0f};

//---- Methods ----

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

void genInstancingVal()
{
    //---- Based on : https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
    // Adapted to glm::vec3 using ChatGPT

    // Remove doubles
    // Creating a comparator
    auto vec3_less = [](const glm::vec3& a, const glm::vec3& b) {
        if (a.x != b.x) return a.x < b.x;
        if (a.y != b.y) return a.y < b.y;
        return a.z < b.z;
    };

    // Sort vector
    std::sort(pos_vector.begin(), pos_vector.end(), vec3_less);

    // Use std::unique to find doubles
    auto it = std::unique(pos_vector.begin(), pos_vector.end(), [](const glm::vec3& a, const glm::vec3& b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    });

    // Remove the doubles
    pos_vector.erase(it, pos_vector.end());

    //----

    // Shuffle the vector
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pos_vector.begin(), pos_vector.end(),g);

    // Put the positions from pos_vector in the grid_pos
    size_t index = 0;
    for (const auto& vec : pos_vector) {
        grid_pos[index++] = vec.x;
        grid_pos[index++] = vec.y;
        grid_pos[index++] = vec.z;
    };

    // Get values from grid_pos
    std::memcpy(grass_pos_2,    grid_pos + 0,                   grassAmount * 3 * sizeof(GLfloat));
    std::memcpy(grass_pos_3,    grid_pos + grassAmount*3,   grassAmount * 3 * sizeof(GLfloat));
    std::memcpy(grass_pos_41,   grid_pos + grassAmount*6,   grassAmount * 3 * sizeof(GLfloat));
    std::memcpy(grass_pos_42,   grid_pos + grassAmount*9,  grassAmount * 3 * sizeof(GLfloat));

    // Init scale and angle
    for (int i=0; i < grassAmount; i++)
    {
        grass_scl[i] = (95 + rand() % 10)/100.0f;
        grass_angl[i] = (-600 + rand() % 1200)/10.0f;
    }

    // Generate new flower positions
    int z = 9;
    int fIndex = 0;
    for (int i=0; i < 4; ++i)
    {
        if (i >=2){z-=2;}
        for (int j=0; j < z; ++j)
        {
            flowers_pos[fIndex++] = i*7;
            flowers_pos[fIndex++] = 0;
            flowers_pos[fIndex++] = j*7 - (z-1)*3.5;

            if (i > 0)
            {
                flowers_pos[fIndex++] = -i*7;
                flowers_pos[fIndex++] = 0;
                flowers_pos[fIndex++] = j*7 - (z-1)*3.5;
            }
        }
    }

    // Init flower angle and scale
    for (int i = 0; i < flowerAmount; ++i)
    {
        flowers_scl[i] = (95 + rand() % 30)/100.0f;
        flowers_angl[i] = (-600 + rand() % 1200)/10.0f;
    }
}


void prepShips(std::map<std::string,GLuint> shaders, gltfObj ships[3],int blockBindFloor)
{
    std::string names[3] = {"virgo","scorpio","gemini"};
    float scales[3] = {8.0f,6.0f,5.0f};

    for (int i = 0; i < 3; ++i)
    {
        std::string modelPath = "../assets/models/ships/" + names[i] + ".gltf";
        std::string texturePath = "../assets/textures/ships/" + names[i] + ".png";

        ships[i].init_s();
        ships[i].init_plmt(glm::vec3(-2500.0f,0.0f,0.0f),glm::vec3(worldScale*scales[i]),glm::vec3(0.0f),0.0f);
        ships[i].init(shaders["obj_def"],shaders["obj_dpth"],i + blockBindFloor,modelPath.c_str(), texturePath.c_str());
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

        glm::vec3 mvtFB = moveDist * dirVect;
        glm::vec3 mvtLR = moveDist * dirSide;

        // Forward
        if (key == GLFW_KEY_Z || key == GLFW_KEY_W)
        {
            if (glm::length(eye_center + mvtFB) < domeBoundIn + tolerance)
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
            }
        }
        // Left
        if (key == GLFW_KEY_Q || key == GLFW_KEY_A)
        {
            if (glm::length(eye_center + mvtLR) < domeBoundIn + tolerance)
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
