#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>


// Other relevant structs

// Each VAO corresponds to each mesh primitive in the GLTF model
struct PrimitiveObject {
    GLuint vao;
    std::map<int, GLuint> vbos;
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

#ifndef MYBOT_H
#define MYBOT_H


struct myBot {

    // Constructors
    myBot();
    ~myBot();

    // Methods

    // Base use fonctions used outside struct
    void initialize(GLuint programID,const char *filename);
    void update(float time);
    void render(glm::mat4 cameraMatrix,glm::vec3 lightPosition,glm::vec3 lightIntensity);
    void cleanup();

    // Nodes computations
    glm::mat4 getNodeTransform(const tinygltf::Node& node);
    void computeLocalNodeTransform(const tinygltf::Model& model, int nodeIndex, std::vector<glm::mat4> &localTransforms);
    void computeGlobalNodeTransform(const tinygltf::Model& model, const std::vector<glm::mat4> &localTransforms, int nodeIndex, const glm::mat4& parentTransform, std::vector<glm::mat4> &globalTransforms);

    // Prepare loading
    std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);
    std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);

    // Loading
    bool loadModel(tinygltf::Model &model, const char *filename);

    // Binding
    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,tinygltf::Model &model, tinygltf::Mesh &mesh);
    void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,tinygltf::Model &model,tinygltf::Node &node);
    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

    // Updates fonctions
    void updateAnimation(const tinygltf::Model &model, const tinygltf::Animation &anim, const AnimationObject &animationObject, float time, std::vector<glm::mat4> &nodeTransforms);
    void updateSkinning(const std::vector<glm::mat4> &nodeTransforms);

    // Draw functions
    void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects, tinygltf::Model &model, tinygltf::Mesh &mesh);
    void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model &model, tinygltf::Node &node);
    void drawModel(const std::vector<PrimitiveObject>& primitiveObjects, tinygltf::Model &model);

    // Helper functions
    int findKeyframeIndex(const std::vector<float>& times, float animationTime);

    // Variables

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint jointMatricesID;
    GLuint ubo_jointMatricesID;
    GLuint lightPositionID;
    GLuint lightIntensityID;
    GLuint programID;

    // Model related variables
    tinygltf::Model model;
    std::vector<PrimitiveObject> primitiveObjects;
    std::vector<SkinObject> skinObjects;
    std::vector<AnimationObject> animationObjects;
};



#endif //MYBOT_H
