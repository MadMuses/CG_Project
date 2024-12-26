#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

// Joint matrices IDs and weight
layout(location = 3) in vec4 j_IDs;
layout(location = 4) in vec4 j_weights;

// Model matrice because of instancing
layout(location = 5) in mat4 i_modelMat;

// View matrices
uniform mat4 MVP;

// vector containing all of the joint matrices
layout(std140) uniform jointMatrices {
    mat4 jointMatricesVec[25];
};

void main() {
    // normalising the weights in case
    float total_weight = j_weights.x + j_weights.y + j_weights.z + j_weights.w;
    vec4 normweights = vec4(j_weights/total_weight);

    // Calculate the skin matrix :
    mat4 skinMat =
    jointMatricesVec[int(j_IDs.x)]* normweights.x
    + jointMatricesVec[int(j_IDs.y)]* normweights.y
    + jointMatricesVec[int(j_IDs.z)]* normweights.z
    + jointMatricesVec[int(j_IDs.w)]* normweights.w;

    // Transform vertex
    gl_Position =  MVP * i_modelMat * skinMat * vec4(vertexPosition,1);
}