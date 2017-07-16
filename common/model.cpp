#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <model.hpp>

sprt_tex2d_t texture_types_supported[TEX_NTexType] = {
	{aiTextureType_AMBIENT, TEX_Ambient},
	{aiTextureType_DIFFUSE, TEX_Diffuse},
	{aiTextureType_NORMALS, TEX_Normal},
	{aiTextureType_SPECULAR, TEX_Specular}
};

GLuint
loadTexture2GPU(const std::string fname)
{
	GLuint tid;

	glGenTextures(1, &tid);
	glBindTexture(GL_TEXTURE_2D, tid);
	cv::Mat img = cv::imread(fname, CV_LOAD_IMAGE_COLOR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Paramaters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tid;
}


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
		this->indices.resize(nnodes / 3);
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
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int
Model::processNode(const aiScene *scene, aiNode *node)
{
	int count = node->mNumMeshes;
	// Process all the node's meshes (if any)
	for(GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(Mesh(scene, mesh));
	}
	// Then do the same for each of its children
	for(GLuint i = 0; i < node->mNumChildren; i++)
	{
		count += this->processNode(scene, node->mChildren[i]);
	}
	return count;
	
}

/**
 * @brief model constructor, loading meshes and textures, 
 * 
 * It does flags to the assimp, so we will get the triangluated mesh for sure
 * In terms of 
 */
Model::Model(const std::string& file, Parameter param)
{
	Assimp::Importer import;
	import.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
	//import.SetPropertyInteger(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
	import.SetPropertyInteger(AI_CONFIG_IMPORT_TER_MAKE_UVS, 1);
	
	unsigned int ppsteps = aiProcess_CalcTangentSpace | // calculate tangents and bitangents 
		aiProcess_JoinIdenticalVertices    | // join identical vertices/ optimize indexing
		aiProcess_ValidateDataStructure    | // perform a full validation of the loader's output
		aiProcess_ImproveCacheLocality     | // improve the cache locality of the output vertices
		aiProcess_RemoveRedundantMaterials | // remove redundant materials
		aiProcess_FindDegenerates          | // remove degenerated polygons from the import
		aiProcess_FindInvalidData          | // detect invalid model data, such as invalid normal vectors
		aiProcess_GenUVCoords              | // convert spherical, cylindrical, box and planar mapping to proper UVs
		aiProcess_FindInstances            | // search for instanced meshes and remove them by references to one master
		aiProcess_LimitBoneWeights         | // limit bone weights to 4 per vertex
		aiProcess_OptimizeMeshes           | // join small meshes, if possible;
		aiProcess_SplitByBoneCount         | // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
		aiProcess_SortByPType              | // sort primitives		
		aiProcess_Triangulate              | // split polygons into triangulate
		aiProcess_FlipUVs                  | // flipUVccordinates 
		0;
	
	const aiScene *scene = import.ReadFile(file, ppsteps);
	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		throw std::runtime_error(std::string("ERROR::ASSIMP") + import.GetErrorString());
	this->root_path = file.substr(0, file.find_last_of('/'));

	//processed mesh
	int count = processNode(scene, scene->mRootNode);
	for (GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].pushMesh2GPU();
	
	std::map<std::string, GLuint> textures_cache;
	//remember, every mesh has a material index
	if (param & NO_TEXTURE)
		return;
	this->Materials.resize(scene->mNumMaterials);
	for (GLuint i = 0; i < scene->mNumMaterials; i++) {
		std::cout << "material indx: " << i << std::endl;
		aiMaterial *mat = scene->mMaterials[i];
		aiString path;
		Material material;
		GLuint gpu_handle;
		for (GLuint j = 0; j < TEX_NTexType; j++) {
			if (mat->GetTextureCount(texture_types_supported[j].aiTextype) > 0) {
				mat->GetTexture(texture_types_supported[j].aiTextype, 0, &path);
				std::string full_path = this->root_path + "/" + std::string(path.C_Str());
				//check whether we loaded already
				auto it = textures_cache.find(full_path);
				if (it == textures_cache.end()) {
					gpu_handle = loadTexture2GPU(full_path);
					textures_cache.insert(std::make_pair(full_path, gpu_handle));
				} else
					gpu_handle = it->second;
				material.push_back(
					Texture(gpu_handle,
						texture_types_supported[j].ourTextype));
			}
		}
		this->Materials[i] = material;
	}
}


Model::Model()
{
	
}

Model::~Model()
{
	//deallocate the materials
	for (unsigned int i = 0; i < this->Materials.size(); i++);
}

void
Model::draw()
{
	const ShaderMan *sm = this->shader_to_draw;
	if (!sm)
		throw std::runtime_error("no shader to draw");
	for (GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].draw(sm, *this);
}

//well, you will need different types of mesh this time
CubeModel::CubeModel()
{
	
	
}
