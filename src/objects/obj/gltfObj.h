#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

// Other relevant structs
#include "../commonStructs.h"
#include "helpers.h"

#ifndef GLTFOBJ_H
#define GLTFOBJ_H

struct gltfObj {

    // Constructors
    gltfObj();
    ~gltfObj();

    // Methods

    // Base use fonctions

    void init_s();
    void init_a();
    void init_plmt(glm::vec3 position, glm::vec3 scale, glm::vec3 rotationAxis,GLfloat rotationAngle);
    void init_i(GLuint amount, GLfloat *pos_i, GLfloat *scale_i, GLfloat *rotAngl_i);

    virtual void init(GLuint programID, GLuint depthProgramID, int blockBindID, const char *filename,const char *texturePath);
    void cleanup();

    // Render methods
    void render(glm::mat4 cameraMatrix, glm::vec3 lightPosition,glm::vec3 lightIntensity, glm::mat4 lightMatrix = glm::mat4(0.0f), GLuint depthTexture = 0);
    void depthRender(glm::mat4 lightViewMatrix);

    // Nodes computations
    glm::mat4 getNodeTransform(const tinygltf::Node& node);
    void computeLocalNodeTransform(const tinygltf::Model& model, int nodeIndex, std::vector<glm::mat4> &localTransforms);
    void computeGlobalNodeTransform(const tinygltf::Model& model, const std::vector<glm::mat4> &localTransforms, int nodeIndex, const glm::mat4& parentTransform, std::vector<glm::mat4> &globalTransforms);

    // Updates fonctions
    void update(float time);
    void updateSkinning(const std::vector<glm::mat4> &nodeTransforms);
    void updateAnimation(const tinygltf::Model &model, const tinygltf::Animation &anim, const AnimationObject &animationObject, float time, std::vector<glm::mat4> &nodeTransforms);

    // Prepare loading
    std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);
    std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);

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

    // helpers functions
    void genModelMat();
    int findKeyframeIndex(const std::vector<float>& times, float animationTime);

    // Variables

    // State of possible activated stuff
    bool shadowsON = false;
    bool instancingON = false;
    bool animationON = false;

    // Basic translation and scale values
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::vec3 rotationAxis = glm::vec3(0.0f);
    GLfloat rotationAngle = 0.0f;

    // Shader variable IDs
    GLuint blockBindID;
    GLuint mvpMatrixID;
    GLuint jointMatricesID;
    GLuint ubo_jointMatricesID;
    GLuint lightPositionID;
    GLuint lightIntensityID;

    // Shader programs
    GLuint programID;
    GLuint depthProgramID;

    // Shadow manipulations
    GLuint lvpMatrixID;
    GLuint depthTextureSamplerID;

    // Instanced
    GLuint instanced = 1; // Default value to one instance
    glm::mat4 *modelMat;

    // Instanciation min-max
    GLfloat *pos_i;               // Array of all the i positions offsets
    GLfloat *scale_i;             // "" scale percentage from original scale
    GLfloat *rotationAngle_i;     // "" rotation angles offset

    // Instance buffers data
    GLuint i_modelMatBuffer;

    // Material uniform handler idea
    GLuint materialUniID;
    GLuint metallicUniID;
    GLuint roughnessUniID;

    // Texture handling
    GLuint textureID;
    GLuint textureSamplerID;
    GLuint validTextureTestID;
    GLfloat validTexture = 0.0f;

    // Model related variables
    tinygltf::Model model;
    std::vector<PrimitiveObject> primitiveObjects;
    std::vector<SkinObject> skinObjects;
    std::vector<MaterialObject> materialObjects;
    std::vector<AnimationObject> animationObjects;
};



#endif //GLTFOBJ
