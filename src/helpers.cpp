#include "helpers.h"
#include <tinygltf-2.9.3/stb_image.h>

void printVec(glm::vec3 v)
{
    std::cout << std::endl << "[" << v.x << ", " << v.y << ", " << v.z << "]" << std::endl;
}

GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;

    // Generate an OpenGL texture and make use of it
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    if (img) {
        // Load the image into the current OpenGL texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}

std::map<std::string,GLuint> LoadShaders()
{
    std::map<std::string,GLuint> shaderlist;

    GLuint programID = LoadShadersFromFile("../src/shaders/obj.vert", "../src/shaders/obj.frag");
    GLuint depthProgramID = LoadShadersFromFile("../src/shaders/obj_depth.vert", "../src/shaders/obj_depth.frag");
    GLuint shadowProgramID = LoadShadersFromFile("../src/shaders/obj_shadow.vert", "../src/shaders/obj_shadow.frag");

    if (programID == 0 || depthProgramID == 0 || shadowProgramID == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }
    shaderlist["objBasic"] = programID;
    shaderlist["objDepth"] = depthProgramID;
    shaderlist["objShadow"] = shadowProgramID;

    return shaderlist;
}