#include <tiny_gltf.h>
#include "gltfObj.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

gltfObj::gltfObj(){}

gltfObj::~gltfObj() {
	cleanup();
}

// Get node transforms from node
glm::mat4 gltfObj::getNodeTransform(const tinygltf::Node& node) {
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

// Compute local transforms and fill it with the node transforms
void gltfObj::computeLocalNodeTransform(const tinygltf::Model& model,
	int nodeIndex,
	std::vector<glm::mat4> &localTransforms)
{
	const tinygltf::Node &node = model.nodes[nodeIndex];
	localTransforms[nodeIndex] = getNodeTransform(node);

	for (const int childIndex : node.children)
	{
		computeLocalNodeTransform(model, childIndex,localTransforms);
	};
}

// Compute global transforms and fill it with the node transforms
void gltfObj::computeGlobalNodeTransform(const tinygltf::Model& model,
	const std::vector<glm::mat4> &localTransforms,
	int nodeIndex, const glm::mat4& parentTransform,
	std::vector<glm::mat4> &globalTransforms)
{
	globalTransforms[nodeIndex] = parentTransform * localTransforms[nodeIndex];

	for (const int childIndex : model.nodes[nodeIndex].children) {
		computeGlobalNodeTransform(model, localTransforms, childIndex,globalTransforms[nodeIndex],globalTransforms);
	};
}

// Create the skinObject vector and fill it with the created skin objects
std::vector<SkinObject> gltfObj::prepareSkinning(const tinygltf::Model &model) {
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

		// Compute joint matrices

		// Get the root node
		int rootNodeIndex = skin.joints[0];

		// Compute the local node transforms
		std::vector<glm::mat4> localNodeTransforms(skin.joints.size());
		computeLocalNodeTransform(model, rootNodeIndex, localNodeTransforms);

		// Compute the global node transforms
		computeGlobalNodeTransform(model, localNodeTransforms, rootNodeIndex,glm::mat4(1.0f),
			skinObject.globalJointTransforms);

		// Calculate the joint matrices
		for (size_t h = 0; h < skinObject.jointMatrices.size(); h++)
		{
			int nodeIndex = skin.joints[h];
			skinObject.jointMatrices[h] = skinObject.globalJointTransforms[nodeIndex] * skinObject.inverseBindMatrices[h];
		}

		skinObjects.push_back(skinObject);
	}
	return skinObjects;
}

// Load the model
bool gltfObj::loadModel(tinygltf::Model &model, const char *filename) {
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

// Get material from model and push it to the materialObjects vector
std::vector<MaterialObject> gltfObj::bindMaterials(tinygltf::Model &model)
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

// Generate vbos and vaas according to the model
void gltfObj::bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
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

// Bind mesh of every node
void gltfObj::bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
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

std::vector<PrimitiveObject> gltfObj::bindModel(tinygltf::Model &model) {
	std::vector<PrimitiveObject> primitiveObjects;

	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
		bindModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}

	return primitiveObjects;
}

void gltfObj::drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
			tinygltf::Model &model, tinygltf::Mesh &mesh) {

	for (size_t i = 0; i < mesh.primitives.size(); ++i)
	{
		GLuint vao = primitiveObjects[i].vao;
		std::map<int, GLuint> vbos = primitiveObjects[i].vbos;

		glBindVertexArray(vao);

		if (instancingON)
		{
			// Send the instantiation matrices
			std::size_t rowSize = sizeof(glm::vec4);

			glBindBuffer(GL_ARRAY_BUFFER, i_modelMatBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, instanced * sizeof(glm::mat4), &modelMat[0]);

			// We use 4 indexes because the maximum amount of possible data per index is 4 (vec4), it will still be received as mat4 in 5
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * rowSize, (void*)0);

			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * rowSize, (void*)(1 * rowSize));

			glEnableVertexAttribArray(7);
			glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * rowSize, (void*)(2 * rowSize));

			glEnableVertexAttribArray(8);
			glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * rowSize, (void*)(3 * rowSize));

			// This tells us that each matrix will be used for each instance
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
			glVertexAttribDivisor(7, 1);
			glVertexAttribDivisor(8, 1);
		}

		tinygltf::Primitive primitive = mesh.primitives[i];
		tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

		// Send material info
		glUniform4fv(materialUniID, 1, &primitiveObjects[i].material.BaseColorFactor[0]);
		glUniform1fv(metallicUniID, 1, &primitiveObjects[i].material.MetallicFactor);
		glUniform1fv(roughnessUniID, 1, &primitiveObjects[i].material.RoughnessFactor);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));

		// Draw with instancing if there is, draws normally if not
		if (instancingON)
		{
			glDrawElementsInstanced(primitive.mode, indexAccessor.count,
						indexAccessor.componentType,
						BUFFER_OFFSET(indexAccessor.byteOffset),
						instanced);
		} else
		{
			glDrawElements(primitive.mode, indexAccessor.count,
			indexAccessor.componentType,
			BUFFER_OFFSET(indexAccessor.byteOffset)
			);
		}
		glBindVertexArray(0);
	}
}

void gltfObj::drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
					tinygltf::Model &model, tinygltf::Node &node) {
	// Draw the mesh at the node, and recursively do so for children nodes
	if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
		drawMesh(primitiveObjects, model, model.meshes[node.mesh]);
	}
	for (size_t i = 0; i < node.children.size(); i++) {
		drawModelNodes(primitiveObjects, model, model.nodes[node.children[i]]);
	}
}
void gltfObj::drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
			tinygltf::Model &model) {
	// Draw all nodes
	const tinygltf::Scene &scene = model.scenes[model.defaultScene];
	for (size_t i = 0; i < scene.nodes.size(); ++i) {
		drawModelNodes(primitiveObjects, model, model.nodes[scene.nodes[i]]);
	}
}

void gltfObj::init(GLuint programID, GLuint depthProgramID, int blockBindID, const char *filename,const char *texturePath) {

	// Modify your path if needed
	if (!loadModel(model, filename)) {
		return;
	}

	// Generate the modelMat if there is no instancing
	if (!instancingON)
	{
		modelMat = new glm::mat4[instanced];
		genModelMat(position,scale);
	}

	// Prepare materials for meshes
	materialObjects = bindMaterials(model);

	// Prepare buffers for rendering
	primitiveObjects = bindModel(model);

	// Prepare joint matrices
	skinObjects = prepareSkinning(model);

	// Get shader program
	this -> programID = programID;
	this -> blockBindID = blockBindID;

	// Get a handle for GLSL variables
	mvpMatrixID = glGetUniformLocation(programID, "MVP");
	lightPositionID = glGetUniformLocation(programID, "lightPosition");
	lightIntensityID = glGetUniformLocation(programID, "lightIntensity");

	materialUniID = glGetUniformLocation(programID, "baseColorFactor");
	metallicUniID = glGetUniformLocation(programID, "metallicFactor");
	roughnessUniID = glGetUniformLocation(programID, "roughnessFactor");

	// Handling textures
	if (texturePath != NULL){
		textureID = LoadTextureTileBox(texturePath);
		textureSamplerID  = glGetUniformLocation(programID,"textureSampler");
		this -> validTexture = 1.0f;
	}
	validTextureTestID = glGetUniformLocation(programID, "validTexture");

	// Generate what's necessary for the passage of the jointMatrices to the shader
	glGenBuffers(1, &jointMatricesID);
	glBindBuffer(GL_UNIFORM_BUFFER, jointMatricesID);
	glBufferData(GL_UNIFORM_BUFFER, skinObjects[0].jointMatrices.size() * sizeof(glm::mat4), skinObjects[0].jointMatrices.data(), GL_DYNAMIC_DRAW);

	// Creating a uniform block index
	ubo_jointMatricesID = glGetUniformBlockIndex(programID, "jointMatrices");
	glUniformBlockBinding(programID, ubo_jointMatricesID, blockBindID);  // 0 est le binding point du UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, blockBindID, jointMatricesID);

	// If shadows are enables, allocate the necessary resources
	if (shadowsON)
	{
		// Setting up variables
		this -> depthProgramID = depthProgramID;

		// Depth texture
		depthTextureSamplerID  = glGetUniformLocation(programID,"depthTextureSampler");

		// Handle for variables
		lvpMatrixID = glGetUniformLocation(programID, "LVP");
	}

	// Creates the necessary animating elements if they are enabled
	if (animationON)
	{
		// Prepare animation data
		animationObjects = prepareAnimation(model);
	}
}

// Init the position,scale,rotation angle and axis, must be used first, NECESSARY
void gltfObj::init_plmt(glm::vec3 position, glm::vec3 scale, glm::vec3 rotationAxis,GLfloat rotationAngle)
{
	// Basic transforms
	this -> position = position;
	this -> scale = scale;
	this -> rotationAxis = rotationAxis;
	this -> rotationAngle = rotationAngle;
}

// Init modulation factor used for downscaling position and scale with it
void gltfObj::init_plmt_mod(GLfloat posMod,GLfloat scaleMod)
{
	// Basic transforms
	this -> posMod = posMod;
	this -> scaleMod = scaleMod;
}

// Tells the object that shadows are wanted
void gltfObj::init_s()
{
	// Setting up variables
	this -> shadowsON = true;
}

// Tells the object it's animation will be played
void gltfObj::init_a()
{
	this -> animationON = true;
}

// Initialise instancing, expects the offset datas (pos_i, scale_i, rotAngle_i) to be respectively 3*"amount", "amount" and "amount" long to work
void gltfObj::init_i(GLuint amount, GLfloat *pos_i, GLfloat *scale_i, GLfloat *rotAngl_i)
{
	// Setting up variables
	this -> instancingON = true;
	this -> instanced = amount;

	// Setup the offsets
	this -> pos_i = pos_i;
	this -> scale_i = scale_i;
	this -> rotationAngle_i = rotAngl_i;

	// Getting the model matrix for each instance
	modelMat = new glm::mat4[amount];
	genModelMat(position,scale);

	// Putting the data in the buffers
	glGenBuffers(1, &i_modelMatBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, i_modelMatBuffer);
	glBufferData(GL_ARRAY_BUFFER, instanced * sizeof(glm::mat4), &modelMat[0], GL_DYNAMIC_DRAW);
}

// Used to generate model matrices using a given position and scale, will create a single matrix in modelMat[0] if there is no instancing
void gltfObj::genModelMat(glm::vec3 position,glm::vec3 scale)
{
	if (instancingON)
	{
		for(unsigned int i = 0; i < instanced*3; i += 3)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, position + glm::vec3(pos_i[i], pos_i[i+1],pos_i[i+2]));
			model = glm::scale(model, scale * scale_i[i/3]);
			model = glm::rotate(model, glm::radians(rotationAngle + rotationAngle_i[i/3]), rotationAxis); // NB : No check here, we have only one rotation axis and need to allow rotation 0

			modelMat[i/3] = model;
		}
	}else
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model,position);																	// Translate the box to its position
		model = glm::scale(model, scale);																		// Scale the box along each axis
		if (rotationAxis != glm::vec3(0.0f))
		{
			model = glm::rotate(model, glm::radians(rotationAngle), rotationAxis);								// Rotate the box along the chosen axis if one is chosen
		}
		modelMat[0] = model;
	}
}

// Render made to give information to the depth buffer only, will not output visuals, very minimal
void gltfObj::depthRender(glm::mat4 lightViewMatrix) {
	glUseProgram(depthProgramID);

	// Set transforms

	// Change the data of the model matrix(es)
	genModelMat(position*posMod,scale*scaleMod);

	if (instancingON)
	{
		// Set camera
		glm::mat4 mvp = lightViewMatrix;
		glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	}
	else
	{
		// Set camera
		glm::mat4 mvp = lightViewMatrix*modelMat[0];
		glUniformMatrix4fv(glGetUniformLocation(depthProgramID, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	}

	// Use the relevant blockBind buffer
	glUniformBlockBinding(depthProgramID, glGetUniformBlockIndex(depthProgramID, "jointMatrices"), blockBindID);  // 0 est le binding point du UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, blockBindID, jointMatricesID);

	// Get the data into the buffer for access in the shaders
	glBindBuffer(GL_UNIFORM_BUFFER, jointMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,skinObjects[0].jointMatrices.size() * sizeof(glm::mat4), skinObjects[0].jointMatrices.data());

	// Draw the GLTF model
	drawModel(primitiveObjects, model);
}

// Main render, lightMatrix and depthTexture will not be used if shadows are not activated but are still required
void gltfObj::render(glm::mat4 cameraMatrix, glm::vec3 lightPosition,glm::vec3 lightIntensity, glm::mat4 lightMatrix, GLuint depthTexture)
{
	glUseProgram(programID);

	// Change the data of the model matrix(es)
	genModelMat(position*posMod,scale*scaleMod);

	if (instancingON)
	{
		// Set camera
		glm::mat4 mvp = cameraMatrix;
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		if (shadowsON)
		{
			// Set light
			glm::mat4 lvp = lightMatrix;
			glUniformMatrix4fv(lvpMatrixID, 1, GL_FALSE, &lvp[0][0]);
		}
	} else
	{
		// Set camera
		glm::mat4 mvp = cameraMatrix*modelMat[0];
		glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);

		if (shadowsON)
		{
			// Set light
			glm::mat4 lvp = lightMatrix*modelMat[0];
			glUniformMatrix4fv(lvpMatrixID, 1, GL_FALSE, &lvp[0][0]);
		}
	}

	// Use the relevant blockBind buffer
	glUniformBlockBinding(programID, ubo_jointMatricesID, blockBindID);  // 0 est le binding point du UBO
	glBindBufferBase(GL_UNIFORM_BUFFER, blockBindID, jointMatricesID);

	// Get the data into the buffer for access in the shaders
	glBindBuffer(GL_UNIFORM_BUFFER, jointMatricesID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0,skinObjects[0].jointMatrices.size() * sizeof(glm::mat4), skinObjects[0].jointMatrices.data());
	// -----------------------------------------------------------------
	// Handling texture

	if (shadowsON)
	{
		// Set depthBuffer Texture data
		glActiveTexture(GL_TEXTURE0+7);
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glUniform1i(depthTextureSamplerID, 7);
	}

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

void gltfObj::cleanup() {
	glDeleteProgram(programID);
}

int gltfObj::findKeyframeIndex(const std::vector<float>& times, float animationTime)
{
	int left = 0;
	int right = times.size() - 1;

	while (left <= right) {
		int mid = (left + right) / 2;

		if (mid + 1 < times.size() && times[mid] <= animationTime && animationTime < times[mid + 1]) {
			return mid;
		}
		else if (times[mid] > animationTime) {
			right = mid - 1;
		}
		else { // animationTime >= times[mid + 1]
			left = mid + 1;
		}
	}

	// Target not found
	return times.size() - 2;
}

std::vector<AnimationObject> gltfObj::prepareAnimation(const tinygltf::Model &model)
{
	std::vector<AnimationObject> animationObjects;
	for (const auto &anim : model.animations) {
		AnimationObject animationObject;

		for (const auto &sampler : anim.samplers) {
			SamplerObject samplerObject;

			const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
			const tinygltf::BufferView &inputBufferView = model.bufferViews[inputAccessor.bufferView];
			const tinygltf::Buffer &inputBuffer = model.buffers[inputBufferView.buffer];

			assert(inputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
			assert(inputAccessor.type == TINYGLTF_TYPE_SCALAR);

			// Input (time) values
			samplerObject.input.resize(inputAccessor.count);

			const unsigned char *inputPtr = &inputBuffer.data[inputBufferView.byteOffset + inputAccessor.byteOffset];
			const float *inputBuf = reinterpret_cast<const float*>(inputPtr);

			// Read input (time) values
			int stride = inputAccessor.ByteStride(inputBufferView);
			for (size_t i = 0; i < inputAccessor.count; ++i) {
				samplerObject.input[i] = *reinterpret_cast<const float*>(inputPtr + i * stride);
			}

			const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
			const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
			const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

			assert(outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

			const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
			const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

			int outputStride = outputAccessor.ByteStride(outputBufferView);

			// Output values
			samplerObject.output.resize(outputAccessor.count);

			for (size_t i = 0; i < outputAccessor.count; ++i) {

				if (outputAccessor.type == TINYGLTF_TYPE_VEC3) {
					memcpy(&samplerObject.output[i], outputPtr + i * 3 * sizeof(float), 3 * sizeof(float));
				} else if (outputAccessor.type == TINYGLTF_TYPE_VEC4) {
					memcpy(&samplerObject.output[i], outputPtr + i * 4 * sizeof(float), 4 * sizeof(float));
				} else {
					std::cout << "Unsupport accessor type ..." << std::endl;
				}

			}

			animationObject.samplers.push_back(samplerObject);
		}

		animationObjects.push_back(animationObject);
	}
	return animationObjects;
}

void gltfObj::updateAnimation(
	const tinygltf::Model &model,
	const tinygltf::Animation &anim,
	const AnimationObject &animationObject,
	float time,
	std::vector<glm::mat4> &nodeTransforms)
{
	// There are many channels so we have to accumulate the transforms
	for (const auto &channel : anim.channels) {

		int targetNodeIndex = channel.target_node;
		const auto &sampler = anim.samplers[channel.sampler];

		// Access output (value) data for the channel
		const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];
		const tinygltf::BufferView &outputBufferView = model.bufferViews[outputAccessor.bufferView];
		const tinygltf::Buffer &outputBuffer = model.buffers[outputBufferView.buffer];

		// Calculate current animation time (wrap if necessary)
		const std::vector<float> &times = animationObject.samplers[channel.sampler].input;
		float animationTime = fmod(time, times.back());

		// Get animation keyframe
		int keyframeIndex = findKeyframeIndex(times, animationTime);

		const unsigned char *outputPtr = &outputBuffer.data[outputBufferView.byteOffset + outputAccessor.byteOffset];
		const float *outputBuf = reinterpret_cast<const float*>(outputPtr);

		// Creates interpolated position, scale and rotation changes
		float t = (animationTime - times[keyframeIndex]) / (times[keyframeIndex+1] - times[keyframeIndex]);

		if (channel.target_path == "translation") {
			glm::vec3 translation0, translation1;
			memcpy(&translation0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			memcpy(&translation1, outputPtr + (keyframeIndex+1) * 3 * sizeof(float), 3 * sizeof(float));

			glm::vec3 translation = translation0 + t*(translation1-translation0);
			nodeTransforms[targetNodeIndex] = glm::translate(nodeTransforms[targetNodeIndex], translation);
		} else if (channel.target_path == "rotation") {
			glm::quat rotation0, rotation1;
			memcpy(&rotation0, outputPtr + keyframeIndex * 4 * sizeof(float), 4 * sizeof(float));
			memcpy(&rotation1, outputPtr + (keyframeIndex+1) * 4 * sizeof(float), 4 * sizeof(float));

			glm::quat rotation = slerp(rotation0, rotation1, t);
			nodeTransforms[targetNodeIndex] *= glm::mat4_cast(rotation);
		} else if (channel.target_path == "scale") {
			glm::vec3 scale0, scale1;
			memcpy(&scale0, outputPtr + keyframeIndex * 3 * sizeof(float), 3 * sizeof(float));
			memcpy(&scale1, outputPtr + (keyframeIndex+1) * 3 * sizeof(float), 3 * sizeof(float));

			glm::vec3 scale = scale0 + t*(scale1-scale0);
			nodeTransforms[targetNodeIndex] = glm::scale(nodeTransforms[targetNodeIndex], scale);
		}
	}
}

void gltfObj::updateSkinning(const std::vector<glm::mat4> &nodeTransforms) {

	for (size_t i = 0; i < model.skins.size(); i++)
	{
		const tinygltf::Skin &skin = model.skins[i];
		SkinObject &skinObject = skinObjects[0];

		// Get the root node
		int rootNodeIndex = skin.joints[0];

		// Compute the global node transforms
		computeGlobalNodeTransform(model, nodeTransforms, rootNodeIndex,glm::mat4(1.0f),skinObject.globalJointTransforms);

		// Calculate the jointmatrices
		for (size_t h = 0; h < skinObject.jointMatrices.size(); h++)
		{
			int nodeIndex = skin.joints[h];
			skinObject.jointMatrices[h] = skinObject.globalJointTransforms[nodeIndex] * skinObject.inverseBindMatrices[h];
		}
	}
}

void gltfObj::update(float time) {
	if (model.animations.size() > 0) {
		const tinygltf::Animation &animation = model.animations[0];
		const AnimationObject &animationObject = animationObjects[0];

		const tinygltf::Skin &skin = model.skins[0];
		std::vector<glm::mat4> nodeTransforms(skin.joints.size());
		for (size_t i = 0; i < nodeTransforms.size(); ++i) {
			nodeTransforms[i] = glm::mat4(1.0);
		}
		updateAnimation(model, animation, animationObject, time, nodeTransforms);
		updateSkinning(nodeTransforms);
	}

}