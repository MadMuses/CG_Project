#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

// Other relevant structs
#include "../commonStructs.h"
#include "helpers.h"

#ifndef STATICOBJ_H
#define STATICOBJ_H

struct staticObj {

    // Constructors
    staticObj();
    ~staticObj();

    // Methods

    // Base use fonctions
    virtual void initialize(GLuint programID,GLuint depthProgramID, int blockBindID,const char *filename, const char *texturePath = NULL,
        glm::vec3 position = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f),glm::vec3 rotationAxis = glm::vec3(0.0f),GLfloat rotationAngle = 0.0f);
    void cleanup();

    // Render methods
    void render(glm::mat4 cameraMatrix,glm::vec3 lightPosition,glm::vec3 lightIntensity);
    void s_render(glm::mat4 cameraMatrix, glm::mat4 lightMatrix,glm::vec3 lightPosition,glm::vec3 lightIntensity,GLuint depthTexture);
    void depthRender(glm::mat4 lightViewMatrix);

    // Nodes computations
    glm::mat4 getNodeTransform(const tinygltf::Node& node);
    void computeLocalNodeTransform(const tinygltf::Model& model, int nodeIndex, std::vector<glm::mat4> &localTransforms);
    void computeGlobalNodeTransform(const tinygltf::Model& model, const std::vector<glm::mat4> &localTransforms, int nodeIndex, const glm::mat4& parentTransform, std::vector<glm::mat4> &globalTransforms);

    // Prepare loading
    std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);

    // Loading
    bool loadModel(tinygltf::Model &model, const char *filename);

    // Binding
    std::vector<MaterialObject> bindMaterials(tinygltf::Model &model);
    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,tinygltf::Model &model, tinygltf::Mesh &mesh);
    void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,tinygltf::Model &model,tinygltf::Node &node);
    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

    // Draw functions
    void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Mesh &mesh);
    void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model &model, tinygltf::Node &node);
    void drawModel(const std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model &model);

    // Variables

    // Basic translation and scale values
    glm::vec3 scale;
    glm::vec3 position;
    glm::vec3 rotationAxis;
    GLfloat rotationAngle;

    // Shader variable IDs
    GLuint blockBindID;
    GLuint mvpMatrixID;
    GLuint jointMatricesID;
    GLuint ubo_jointMatricesID;
    GLuint howManyJointsID;
    GLuint lightPositionID;
    GLuint lightIntensityID;

    // Shader programs
    GLuint programID;
    GLuint depthProgramID;

    // Shadow manipulations
    GLuint lvpMatrixID;
    GLuint depthTextureSamplerID;

    // Material uniform handler idea
    GLuint materialUniID;
    GLuint metallicUniID;
    GLuint roughnessUniID;

    // Texture handling
    GLuint textureID;
    GLuint textureSamplerID;
    GLuint validTextureTestID;
    GLfloat validTexture;

    // Model related variables
    tinygltf::Model model;
    std::vector<PrimitiveObject> primitiveObjects;
    std::vector<SkinObject> skinObjects;
    std::vector<MaterialObject> materialObjects;
};



#endif //STATICOBJ_H
