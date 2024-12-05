#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

layout(location = 3) in vec4 j_IDs;
layout(location = 4) in vec4 j_weights;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;

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

    mat4 skinMatNormal = transpose(inverse(skinMat));

    // World-space geometry
    worldPosition = (MVP * skinMat * vec4(vertexPosition,1)).xyz;
    worldNormal = normalize(skinMatNormal * vec4(vertexNormal, 0.0)).xyz;

    // Transform vertex
    gl_Position =  MVP * skinMat * vec4(vertexPosition,1);
}
