#ifndef HELPERS_H
#define HELPERS_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <iostream>
#include <math.h>
#include <iomanip>



GLFWwindow* initOpenGL(int windowWidth, int windowHeight);
void printVec(glm::vec3 v);
GLuint LoadTextureTileBox(const char *texture_file_path);

#endif //HELPERS_H
