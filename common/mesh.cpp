#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <Eigen/Core>
#include <Eigen/Sparse>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Eigen/Core>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <model.hpp>
#include <data.hpp>


Mesh::Mesh(const aiScene *scene, aiMesh *mesh)
{
	//rebind for easier name
	std::vector<glm::vec3> &poses = this->vertices.Positions;
	std::vector<glm::vec3> &norms = this->vertices.Normals;
	std::vector<glm::vec2> &texuvs= this->vertices.TexCoords;

	poses.resize(mesh->mNumVertices);
	norms.resize(mesh->mNumVertices);
	if (mesh->mTextureCoords[0])
		texuvs.resize(mesh->mNumVertices);
	
	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		poses[i] = glm::vec3(mesh->mVertices[i].x,
				     mesh->mVertices[i].y,
				     mesh->mVertices[i].z);
		norms[i] = glm::vec3(mesh->mNormals[i].x,
				     mesh->mNormals[i].y,
				     mesh->mNormals[i].z);
		if (mesh->mTextureCoords[0])
			texuvs[i] = glm::vec2(mesh->mTextureCoords[0][i].x,
					      mesh->mTextureCoords[0][i].y);
	}
	this->indices.resize(mesh->mNumFaces * 3);
	for (GLuint i = 0; i < mesh->mNumFaces; i++) {
		//one face should only have 3 indices
		aiFace face = mesh->mFaces[i];
		indices[3*i]   = face.mIndices[0];
		indices[3*i+1] = face.mIndices[1];
		indices[3*i+2] = face.mIndices[2];
	}
	this->materialIndx = (int)mesh->mMaterialIndex;

	/*
	if (mesh->mNumBones) {
		for (uint i = 0; i < mesh->mNumBones; i++) {
			std::string bone_name = mesh->mBones[i]->mName.data;
		}
	}
	*/
}

//a mesh without color
Mesh::Mesh(const std::vector<glm::vec3>& vertxs,
	       const std::vector<glm::vec3>& norms,
	       const std::vector<float>& indices,
	       const std::vector<glm::vec2>& uvs,
	       const unsigned int material_id)
{
	assert(vertxs.size() == norms.size());
	assert(indices.size() % 3 == 0);
	assert(uvs.size() == vertxs.size() || uvs.size() == 0);
	std::vector<glm::vec3> &poses   = this->vertices.Positions;
	std::vector<glm::vec3> &normals = this->vertices.Normals;
	std::vector<glm::vec2> &texuvs  = this->vertices.TexCoords;

	std::copy(vertxs.begin(), vertxs.end(), poses.begin());
	std::copy(norms.begin(), norms.end(), normals.begin());
	if (uvs.size() > 0)
		std::copy(uvs.begin(), uvs.end(), texuvs.begin());

	std::copy(indices.begin(), indices.end(), this->indices.begin());
	this->materialIndx = material_id; //the model should take care of this
}


Mesh::Mesh(const float *vertx, const float *norms, const float *uvs, const int nnodes,
	     const float *indices, const int nfaces)
{
	const int size_vn = 3;
	const int size_uv = 2;

	std::vector<glm::vec3> &poses   = this->vertices.Positions;
	std::vector<glm::vec3> &normals = this->vertices.Normals;
	std::vector<glm::vec2> &texuvs  = this->vertices.TexCoords;
	poses.resize(nnodes);
	normals.resize(nnodes);
	if (uvs)
		texuvs.resize(nnodes);
	for (int i = 0; i < nnodes; i++) {
		poses[i]   = glm::make_vec3(i*size_vn + vertx);
		normals[i] = glm::make_vec3(i*size_vn + norms);
		texuvs[i]  = glm::make_vec2(i*size_uv + uvs);
	}
	if (indices) {
		this->indices.resize(nfaces*3);
		std::copy(indices, indices + nfaces*3, this->indices.begin());
	} else {
		//otherwise we make a indices as well. so no draw triangles.
		this->indices.resize(nnodes);
		int n = {0};
		std::generate(this->indices.begin(), this->indices.end(), [&] {return n++;});
	}
}

//a mesh uses draw_triangles instead of
Mesh::~Mesh(void)
{
	if (this->VAO) {
		glDeleteVertexArrays(1, &this->VAO);
		this->VAO = 0;
	}
	if (this->VBO) {
		glDeleteBuffers(1, &this->VBO);
		this->VBO = 0;
	}
	if (this->EBO) {
		glDeleteBuffers(1, &this->EBO);
		this->EBO = 0;
	}
}

void
Mesh::pushMesh2GPU(int param)
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
		     &this->indices[0],
		     GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.Positions.size() * (2 * sizeof(glm::vec3) + sizeof(glm::vec2)),
		     NULL, GL_STATIC_DRAW);
	size_t offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, offset, this->vertices.Positions.size() * sizeof(glm::vec3),
			&this->vertices.Positions[0]);
	//Enable Attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
			      (GLvoid *)offset);
	offset += this->vertices.Positions.size() * sizeof(glm::vec3);
	
	if (param & LOAD_NORMAL) {
		glBufferSubData(GL_ARRAY_BUFFER, offset,
				this->vertices.Normals.size() * sizeof(glm::vec3),
				&this->vertices.Normals[0]);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
				      (GLvoid *)offset);
		offset += this->vertices.Normals.size() * sizeof(glm::vec3);
	}
	if (param & LOAD_TEX) {
		glBufferSubData(GL_ARRAY_BUFFER,
				offset, this->vertices.TexCoords.size() * sizeof(glm::vec2),
				&this->vertices.TexCoords[0]);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
				      (GLvoid *)offset);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void
Mesh::draw(const ShaderMan *sm, const Model& model)
{
	GLuint prog = sm->getPid();
	glUseProgram(prog);
	const Material& mat = model.Materials[this->materialIndx];
	// The 2D texture binding should be like this, although we have too loop.
	//it is is at most 4*4 something

	for (GLuint i = 0; i < sm->tex_uniforms.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		for (GLuint j = 0; j < model.Materials[this->materialIndx].size(); j++) {
			if (sm->tex_uniforms[i] == mat[i].type) {
				glBindTexture(GL_TEXTURE_2D, mat[j].id);
				break;
			}
		}
	}
	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	if (model.instanceVBO == 0) {
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	}
	else {
//		std::cerr << model.get_ninstances() << std::endl;
		glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, model.getNinstances());
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void
Mesh::loadBoneWeights(aiMesh *mesh, const Model &model)
{
	typedef Eigen::Triplet<float> T;
	std::vector<T> vertexWeight;
	
	size_t nvex = this->vertices.Positions.size();
	this->boneWeights.resize(nvex, model.bones.size());
	for (uint i = 0; i < mesh->mNumBones; i++) {
		std::string  bone_name = mesh->mBones[i]->mName.data;
		//all the way to the end of the bones 
		uint bone_id = model.bones.find(bone_name)->second.getInd();
		aiBone *b = mesh->mBones[i];
		for (GLuint j = 0; j < b->mNumWeights; j++) {
			uint vid = b->mWeights[j].mVertexId;
			float weight = b->mWeights[j].mWeight;
			vertexWeight.push_back(T(vid, bone_id, weight));
		}
	}
	this->boneWeights.setFromTriplets(vertexWeight.begin(), vertexWeight.end());
//	std::cout << "weights of the mesh" << this->boneWeights.nonZeros() << std::endl;
}
