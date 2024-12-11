#ifndef HELPERS_H
#define HELPERS_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <map>
#include <vector>
#include <iostream>
#include <math.h>
#include <iomanip>
#include <render/shader.h>

GLFWwindow* initOpenGL(int windowWidth, int windowHeight);
void printVec(glm::vec3 v);
GLuint LoadTextureTileBox(const char *texture_file_path);
std::map<std::string,GLuint> LoadShaders();

#endif //HELPERS_H
