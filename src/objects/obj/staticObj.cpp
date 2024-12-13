#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <tiny_gltf.h>

#include <render/shader.h>

#include <vector>
#include <iostream>

#include <math.h>
#include <iomanip>

#include "staticObj.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

staticObj::staticObj(){}

staticObj::~staticObj() {
	cleanup();
}

glm::mat4 staticObj::getNodeTransform(const tinygltf::Node& node) {
	glm::mat4 transform(1.0f);

	if (node.matrix.size() == 16) {
		transform = glm::make_mat4(node.matrix.data());
	} else {
		if (node.translation.size() == 3) {
			transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
		}
		if (node.rotation.size() == 4) {
			glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			transform *= glm::mat4_cast(q);
		}
		if (node.scale.size() == 3) {
			transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
		}
	}
	return transform;
}

void staticObj::computeLocalNodeTransform(const tinygltf::Model& model,
	int nodeIndex,
	std::vector<glm::mat4> &localTransforms)
{
	// ---------------------------------------
	// TODO: your code here
	// ----------------------------------------

	const tinygltf::Node &node = model.nodes[nodeIndex];
	localTransforms[nodeIndex] = getNodeTransform(node);

	for (const int childIndex : node.children)
	{
		computeLocalNodeTransform(model, childIndex,localTransforms);
	};

	// ---------------------------------------
}

void staticObj::computeGlobalNodeTransform(const tinygltf::Model& model,
	const std::vector<glm::mat4> &localTransforms,
	int nodeIndex, const glm::mat4& parentTransform,
	std::vector<glm::mat4> &globalTransforms)
{
	// ----------------------------------------
	// TODO: your code here
	// ----------------------------------------

	globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];

	for (const int childIndex : model.nodes[nodeIndex].children) {
		computeGlobalNodeTransform(model, localTransforms, childIndex,globalTransforms[nodeIndex],globalTransforms);
	};

	// ----------------------------------------
}

std::vector<SkinObject> staticObj::prepareSkinning(const tinygltf::Model &model) {
	std::vector<SkinObject> skinObjects;

	// In our Blender exporter, the default number of joints that may influence a vertex is set to 4, just for convenient implementation in shaders.

	for (size_t i = 0; i < model.skins.size(); i++) {
		SkinObject skinObject;

		const tinygltf::Skin &skin = model.skins[i];

		// Read inverseBindMatrices
		const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
		assert(accessor.type == TINYGLTF_TYPE_MAT4);
		const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
		const float *ptr = reinterpret_cast<const float *>(
            buffer.data.data() + accessor.byteOffset + bufferView.byteOffset);

		skinObject.inverseBindMatrices.resize(accessor.count);
		for (size_t j = 0; j < accessor.count; j++) {
			float m[16];
			memcpy(m, ptr + j * 16, 16 * sizeof(float));
			skinObject.inverseBindMatrices[j] = glm::make_mat4(m);
		}

		assert(skin.joints.size() == accessor.count);

		skinObject.globalJointTransforms.resize(skin.joints.size());
		skinObject.jointMatrices.resize(skin.joints.size());

		// ----------------------------------------------
		// TODO: your code here to compute joint matrices
		// ----------------------------------------------

		// Get the root node
		int rootNodeIndex = skin.joints[0];

		// Compute the local node transforms
		std::vector<glm::mat4> localNodeTransforms(skin.joints.size());
		computeLocalNodeTransform(model, rootNodeIndex, localNodeTransforms);

		// Compute the global node transforms
		computeGlobalNodeTransform(model, localNodeTransforms, rootNodeIndex,glm::mat4(1.0f),
			skinObject.globalJointTransforms);

		// Calculate the jointmatrices
		for (size_t h = 0; h < skinObject.jointMatrices.size(); h++)
		{
			int nodeIndex = skin.joints[h];
			skinObject.jointMatrices[h] = skinObject.globalJointTransforms[nodeIndex] * skinObject.inverseBindMatrices[h];
		}
		// ----------------------------------------------

		skinObjects.push_back(skinObject);
	}
	return skinObjects;
}

bool staticObj::loadModel(tinygltf::Model &model, const char *filename) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	if (!warn.empty()) {
		std::cout << "WARN: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "ERR: " << err << std::endl;
	}

	if (!res)
		std::cout << "Failed to load glTF: " << filename << std::endl;
	else
		std::cout << "Loaded glTF: " << filename << std::endl;

	return res;
}

std::vector<MaterialObject> staticObj::bindMaterials(tinygltf::Model &model)
{
	std::vector<MaterialObject> materialObjects;
	for (size_t i=0; i< model.materials.size();i++)
	{
		const tinygltf::Material &fetchedmaterial = model.materials[i];
		MaterialObject material;
		material.BaseColorFactor = glm::vec4(1.0f);
		// Fixed to 4 as it is RGBa
		for (size_t j =0; j < 4; j++)
		{
			material.BaseColorFactor[j] = fetchedmaterial.pbrMetallicRoughness.baseColorFactor[j];
		}
		material.MetallicFactor = fetchedmaterial.pbrMetallicRoughness.metallicFactor;
		material.RoughnessFactor = fetchedmaterial.pbrMetallicRoughness.roughnessFactor;

		materialObjects.push_back(material);
	}
	return materialObjects;
};

void staticObj::bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
			tinygltf::Model &model, tinygltf::Mesh &mesh) {

	std::map<int, GLuint> vbos;
	for (size_t i = 0; i < model.bufferViews.size(); ++i) {
		const tinygltf::BufferView &bufferView = model.bufferViews[i];

		int target = bufferView.target;

		if (bufferView.target == 0) {
			// The bufferView with target == 0 in our model refers to
			// the skinning weights, for 25 joints, each 4x4 matrix (16 floats), totaling to 400 floats or 1600 bytes.
			// So it is considered safe to skip the warning.
			//std::cout << "WARN: bufferView.target is zero" << std::endl;
			continue;
		}

		const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(target, vbo);
		glBufferData(target, bufferView.byteLength,
					&buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

		vbos[i] = vbo;
	}

	// Each mesh can contain several primitives (or parts), each we need to
	// bind to an OpenGL vertex array object
	for (size_t i = 0; i < mesh.primitives.size(); ++i) {

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		for (auto &attrib : primitive.attributes) {
			tinygltf::Accessor accessor = model.accessors[attrib.second];
			int byteStride =
				accessor.ByteStride(model.bufferViews[accessor.bufferView]);
			glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

			int size = 1;
			if (accessor.type != TINYGLTF_TYPE_SCALAR) {
				size = accessor.type;
			}

			int vaa = -1;
			if (attrib.first.compare("POSITION") == 0) vaa = 0;
			if (attrib.first.compare("NORMAL") == 0) vaa = 1;
			if (attrib.first.compare("TEXCOORD_0") == 0) vaa = 2;
			if (attrib.first.compare("JOINTS_0") == 0) vaa = 3;
			if (attrib.first.compare("WEIGHTS_0") == 0) vaa = 4;
			if (vaa > -1) {
				glEnableVertexAttribArray(vaa);
				glVertexAttribPointer(vaa, size, accessor.componentType,
									accessor.normalized ? GL_TRUE : GL_FALSE,
									byteStride, BUFFER_OFFSET(accessor.byteOffset));
			} else {
				std::cout << "vaa missing: " << attrib.first << std::endl;
			}
		}

		// Record VAO for later use
		PrimitiveObject primitiveObject;
		primitiveObject.vao = vao;
		primitiveObject.vbos = vbos;

		// Fetch current material
		MaterialObject material = materialObjects[primitive.material];

		// Update for first render
		glUniform4fv(materialUniID, 1, &material.BaseColorFactor[0]);
		glUniform1fv(metallicUniID, 1, &material.MetallicFactor);
		glUniform1fv(roughnessUniID, 1, &material.RoughnessFactor);

		// Store material in primitive object
		primitiveObject.material = material;

		// Store in the general vector
		primitiveObjects.push_back(primitiveObject);

		glBindVertexArray(0);
	}
}

void staticObj::bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
					tinygltf::Model &model,
					tinygltf::Node &node) {
	// Bind buffers for the current mesh at the node
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		bindMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}

	// Recursive into children nodes
	for (size_t i = 0; i < node.children.size(); i++) {
		assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}

std::vector<PrimitiveObject> staticObj::bindModel(tinygltf::Model &model) {
	std::vector<PrimitiveObject> primitiveObjects;

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}

	return primitiveObjects;
}

void staticObj::drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
			tinygltf::Model &model, tinygltf::Mesh &mesh) {

	for (size_t i = 0; i < mesh.primitives.size(); ++i)
	{
		GLuint vao = primitiveObjects[i].vao;
		std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

		glBindVertexArray(vao);

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		glUniform4fv(materialUniID, 1, &primitiveObjects[i].material.BaseColorFactor[0]);
		glUniform1fv(metallicUniID, 1, &primitiveObjects[i].material.MetallicFactor);
		glUniform1fv(roughnessUniID, 1, &primitiveObjects[i].material.RoughnessFactor);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

		glDrawElements(primitive.mode, indexAccessor.count,
					indexAccessor.componentType,
					BUFFER_OFFSET(indexAccessor.byteOffset));

		glBindVertexArray(0);
	}
}

void staticObj::drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
					tinygltf::Model &model, tinygltf::Node &node) {
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}
void staticObj::drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
			tinygltf::Model &model) {
	// Draw all nodes
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}
}

void staticObj::initialize(GLuint programID, GLuint depthProgramID, int blockBindID, const char *filename, const char *texturePath,
	glm::vec3 position, glm::vec3 scale, glm::vec3 rotationAxis,GLfloat rotationAngle) {

	// Basic transforms
	this -> position = position;
	this -> scale = scale;
	this -> rotationAxis = rotationAxis;
	this -> rotationAngle = rotationAngle;

	// Modify your path if needed
	if (!loadModel(model, filename)) {
		return;
	}

	// Prepare materials for meshes
	materialObjects = bindMaterials(model);

	// Prepare buffers for rendering
	primitiveObjects = bindModel(model);

	// Prepare joint matrices
	skinObjects = prepareSkinning(model);

	// Get shader program
	this -> programID = programID;
	this -> depthProgramID = depthProgramID;
	this -> blockBindID = blockBindID;

	// Get a handle for GLSL variables
	mvpMatrixID = glGetUniformLocation(programID, "MVP");
	lightPositionID = glGetUniformLocation(programID, "lightPosition");
	lightIntensityID = glGetUniformLocation(programID, "lightIntensity");

	materialUniID = glGetUniformLocation(programID, "baseColorFactor");
	metallicUniID = glGetUniformLocation(programID, "metallicFactor");
	roughnessUniID = glGetUniformLocation(programID, "roughnessFactor");

	// Handling textures
	if (texturePath == NULL){
		this -> validTexture = 0.0f;
	}else{
		textureID = LoadTextureTileBox(texturePath);
		textureSamplerID  = glGetUniformLocation(programID,"textureSampler");
		this -> validTexture = 1.0f;
	}
	validTextureTestID = glGetUniformLocation(programID, "validTexture");

	// Generate what's necessary for the passage of the jointMatrices to the shader
	// NB : To make it more adaptable, we'll need to pass the size of the vector as another uniform to the shader
/*
	howManyJointsID = glGetUniformLocation(programID, "howManyJoints");
	int thatManyJoints = skinObjects[0].jointMatrices.size();
	std::cout << std::endl << thatManyJoints << std::endl;
	glUniform1iv(howManyJointsID, 1, &thatManyJoints);
*/
	glGenBuffers(1, &jointMatricesID);
	glBindBuffer(GL_UNIFORM_BUFFER, jointMatricesID);
	glBufferData(GL_UNIFORM_BUFFER, skinObjects[0].jointMatrices.size() * sizeof(glm::mat4), skinObjects[0].jointMatrices.data(), GL_DYNAMIC_DRAW);

	// Creating a uniform block index
	ubo_jointMatricesID = glGetUniformBlockIndex(programID, "jointMatrices");
	glUniformBlockBinding(programID, ubo_jointMatricesID, blockBindID);  // 0 est le binding point du UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, blockBindID, jointMatricesID);

}

void staticObj::render(glm::mat4 cameraMatrix,glm::vec3 lightPosition,glm::vec3 lightIntensity) {
	glUseProgram(programID);

	// Set transforms
	glm::mat4 modelMatrix = glm::mat4();

	// Translate the box to its position
	modelMatrix = glm::translate(modelMatrix, position);

	// Scale the box along each axis
	modelMatrix = glm::scale(modelMatrix, scale);

	// Rotate the box along the chosen axis
	if (rotationAngle != 0) {
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), rotationAxis);
	}

	// Set camera
	glm::mat4 mvp = cameraMatrix*modelMatrix;
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

	// -----------------------------------------------------------------
	// TODO: Set animation data for linear blend skinning in shader
	// -----------------------------------------------------------------
/*
	int thatManyJoints = skinObjects[0].jointMatrices.size();
	std::cout << std::endl << thatManyJoints << std::endl;
	glUniform1iv(howManyJointsID, 1, &thatManyJoints);
*/
	// Use the relevant blockBind buffer
	glUniformBlockBinding(programID, ubo_jointMatricesID, blockBindID);  // 0 est le binding point du UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, blockBindID, jointMatricesID);

	// Get the data into the buffer for access in the shaders
	glBindBuffer(GL_UNIFORM_BUFFER, jointMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,skinObjects[0].jointMatrices.size() * sizeof(glm::mat4), skinObjects[0].jointMatrices.data());
	// -----------------------------------------------------------------

	// Handling texture

	// Send texture through sampler
	glActiveTexture(GL_TEXTURE0 + blockBindID + 10);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(textureSamplerID, blockBindID + 10);

	glUniform1fv(validTextureTestID,1,&validTexture);

	// Set light data
	glUniform3fv(lightPositionID, 1, &lightPosition[0]);
	glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);

	// Draw the GLTF model
	drawModel(primitiveObjects, model);
}

void staticObj::depthRender(glm::mat4 lightViewMatrix) {
	glUseProgram(depthProgramID);

	// Set transforms
	glm::mat4 modelMatrix = glm::mat4();

	// Translate the box to its position
	modelMatrix = glm::translate(modelMatrix, position);

	// Scale the box along each axis
	modelMatrix = glm::scale(modelMatrix, scale);

	// Rotate the box along the chosen axis
	if (rotationAngle != 0) {
		modelMatrix = glm::rotate(modelMatrix, glm::radians(rotationAngle), rotationAxis);
	}

	// Set camera
	glm::mat4 mvp = lightViewMatrix*modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "MVP"), 1, GL_FALSE, &mvp[0][0]);

	// Use the relevant blockBind buffer
	glUniformBlockBinding(depthProgramID, glGetUniformBlockIndex(depthProgramID, "jointMatrices"), blockBindID);  // 0 est le binding point du UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, blockBindID, jointMatricesID);

	// Get the data into the buffer for access in the shaders
	glBindBuffer(GL_UNIFORM_BUFFER, jointMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,skinObjects[0].jointMatrices.size() * sizeof(glm::mat4), skinObjects[0].jointMatrices.data());

	// Draw the GLTF model
	drawModel(primitiveObjects, model);
}

void staticObj::cleanup() {
	glDeleteProgram(programID);
}
