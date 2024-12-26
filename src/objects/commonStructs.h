#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

//------------------------------------------------------------------//
//																	//
//		This file contain a list of all the common structure        //
//  used by the gltfObj structure. They could be integrated in      //
//  the gltfObj.h file but are here for clarity and potential       //
//  future integration purposes.                                    //
//																	//
//------------------------------------------------------------------//


#ifndef COMMONSTRUCTS_H
#define COMMONSTRUCTS_H

// Each primitive contains a given material
struct MaterialObject
{
    glm::vec4 BaseColorFactor;
    GLfloat MetallicFactor;
    GLfloat RoughnessFactor;
};

// Each VAO corresponds to each mesh primitive in the GLTF model
struct PrimitiveObject {
    GLuint vao;
    std::map<int, GLuint> vbos;
    MaterialObject material;
};

// Skinning
struct SkinObject {
    // Transforms the geometry into the space of the respective joint
    std::vector<glm::mat4> inverseBindMatrices;

    // Transforms the geometry following the movement of the joints
    std::vector<glm::mat4> globalJointTransforms;

    // Combined transforms
    std::vector<glm::mat4> jointMatrices;
};

// Animation
struct SamplerObject {
    std::vector<float> input;
    std::vector<glm::vec4> output;
    int interpolation;
};
struct ChannelObject {
    int sampler;
    std::string targetPath;
    int targetNode;
};
struct AnimationObject {
    std::vector<SamplerObject> samplers;	// Animation data
};

#endif //COMMONSTRUCTS_H
