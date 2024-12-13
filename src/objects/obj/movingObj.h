#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <iostream>
#include <iomanip>

#include "staticObj.h"

#ifndef MOVINGOBJ_H
#define MOVINGOBJ_H

struct movingObj : public staticObj {

    // changing the initialise to setup the animation
    void initialize(GLuint programID, GLuint depthProgramID,int blockBindID,const char *filename,const char *texturePath = NULL,
        glm::vec3 position = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f),glm::vec3 rotationAxis = glm::vec3(0.0f),GLfloat rotationAngle = 0.0f) override;

    // Helper
    int findKeyframeIndex(const std::vector<float>& times, float animationTime);

    // Updates fonctions
    void update(float time);
    void updateSkinning(const std::vector<glm::mat4> &nodeTransforms);
    void updateAnimation(const tinygltf::Model &model, const tinygltf::Animation &anim, const AnimationObject &animationObject, float time, std::vector<glm::mat4> &nodeTransforms);

    // Animation related functions
    std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);

    // New necessary variables
    std::vector<AnimationObject> animationObjects;
};



#endif //MOVINGOBJ_H
