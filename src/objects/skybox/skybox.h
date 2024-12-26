#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <render/shader.h>

#ifndef SKYBOX_H
#define SKYBOX_H

//------------------------------------------------------------------//
//																	//
//		This structure allows you to create a skybox object			//
//		Shaders path are implemented inside "initialize" if			//
//		they need to be changed.									//
//																	//
//------------------------------------------------------------------//

struct Skybox {

	void initialize(glm::vec3 scale = glm::vec3(1.0f),glm::vec3 position = glm::vec3(0.0f));
	void render(glm::mat4 cameraMatrix, glm::vec3 scale);
	void cleanup();

	// All transforms
	glm::vec3 position;		// Position of the box
	glm::vec3 scale;		// Size of the box in each axis

	// OpenGL buffers
	GLuint vertexArrayID;
	GLuint vertexBufferID;
	GLuint indexBufferID;
	GLuint colorBufferID;
	GLuint uvBufferID;
	GLuint samplerIndex_buffer_ID;

	// Handling textures (One texture/sampler per face)
	GLuint textureIDs[6];			// All the loaded textures
	GLuint textureSamplerIDs[6];	// All the texture samplers IDs

	// Render var ID
	GLuint mvpMatrixID;

	// Shader variable IDs
	GLuint programID;

	// Data
	const char* texturePaths[6] = {		// Path to the starbox texture
		"../assets/textures/starbox/starbox_front.png",
		"../assets/textures/starbox/starbox_back.png",
		"../assets/textures/starbox/starbox_right.png",
		"../assets/textures/starbox/starbox_left.png",
		"../assets/textures/starbox/starbox_down.png",
		"../assets/textures/starbox/starbox_up.png"
	};

	GLfloat samplerIndex_buffer_data [24] = {	// Tells which texture is used per vertex
		0.0, 0.0, 0.0, 0.0,
		1.0, 1.0, 1.0, 1.0,
		2.0, 2.0, 2.0, 2.0,
		3.0, 3.0, 3.0, 3.0,
		4.0, 4.0, 4.0, 4.0,
		5.0, 5.0, 5.0, 5.0
	};

	GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
		// Front face
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,

		// Back face
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,

		// Left face
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		// Right face
		1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,

		// Top face
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		// Bottom face
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,
	};

	GLfloat color_buffer_data[72] = {
		// Front, red
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,

		// Back, yellow
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,

		// Left, green
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,

		// Right, cyan
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f,

		// Top, blue
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,

		// Bottom, magenta
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f,
	};

	GLuint index_buffer_data[36] = {
		// Front face
		3, 2, 1,
		3, 1, 0,

		// Back face
		7, 6, 5,
		7, 5, 4,

		// Left face
		11, 10, 9,
		11, 9, 8,

		// Right face
		15, 14, 13,
		15, 13, 12,

		// Top face
		19, 18, 17,
		19, 17, 16,

		// Bottom face
		23, 22, 21,
		23, 21, 20,
	};


	GLfloat uv_buffer_data[48] = {
		// Front
		1.0f,1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		// Back
		1.0f,1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		// Left
		1.0f,1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		// Right
		1.0f,1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		// Top
		1.0f,1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

		// Bottom
		1.0f,1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,

	};
    // ---------------------------
};

#endif //SKYBOX_H
