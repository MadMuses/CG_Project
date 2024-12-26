#include "skybox.h"

#include <helpers.h>

void Skybox::initialize(glm::vec3 scale,glm::vec3 position ) {
	// Define scale of the skybox geometry
	this->scale = scale;
	this->position = position;

	// Create a vertex array object
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create a vertex buffer object to store the vertex data
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	// Create a vertex buffer object to store the color data (unused but can be useful for debug)
	glGenBuffers(1, &colorBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

	// Create a vertex buffer object to store the uv data
	glGenBuffers(1, &uvBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

	// Create an index buffer object to store the index data that defines triangle faces
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

	// Create an index buffer object to store the index of the texture used for a face
	glGenBuffers(1, &samplerIndex_buffer_ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, samplerIndex_buffer_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(samplerIndex_buffer_data), samplerIndex_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	programID = LoadShadersFromFile("../src/shaders/skybox.vert", "../src/shaders/skybox.frag");
	if (programID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Get a handle for our "MVP" uniform
	mvpMatrixID = glGetUniformLocation(programID, "MVP");


    // Load the textures
	for (int i = 0; i < 6; i++) {
		textureIDs[i] = LoadTextureTileBox(texturePaths[i]);
	}

    // Get handles for the texture samplers
	for (int i = 0; i < 6; i++)
	{
		textureSamplerIDs[i] = glGetUniformLocation(programID, ("textureSampler" + std::to_string(i)).c_str() );
	}
	glBindVertexArray(0);
}

void Skybox::render(glm::mat4 cameraMatrix, glm::vec3 scale) {

	// Bind the relevant shader and vertex program
	glUseProgram(programID);
	glBindVertexArray(vertexArrayID);

	// Enable attibute array
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

	// Model transform
    glm::mat4 modelMatrix = glm::mat4();
	modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

	// Set model-view-projection matrix
	glm::mat4 mvp = cameraMatrix * modelMatrix;
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

	// Enable UV buffer and texture sampler
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable Facetype buffer
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, samplerIndex_buffer_ID);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0);

	// Send texture face by face
	for (int i = 0; i < 6; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		glUniform1i(textureSamplerIDs[i], i);
	}

    // ------------------------------------------
	// Draw the box
	glDrawElements(
		GL_TRIANGLES,      // mode
		36,    			   // number of indices
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	glBindVertexArray(0);
}

void Skybox::cleanup() {
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteBuffers(1, &colorBufferID);
	glDeleteBuffers(1, &indexBufferID);
	glDeleteVertexArrays(1, &vertexArrayID);
	glDeleteBuffers(1, &uvBufferID);
	glDeleteProgram(programID);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

