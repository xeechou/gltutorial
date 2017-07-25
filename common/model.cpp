#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <assimp/cimport.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <model.hpp>
#include <data.hpp>

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
		glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, model.get_ninstances());
	}

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
	//setup constant
	this->instanceVBO = 0;
	
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
	//setup the meshlayout
	this->n_mesh_layouts = 3;
	
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
	if (this->instanceVBO)
		glDeleteBuffers(1, &this->instanceVBO);
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

void
Model::draw(const ShaderMan *different_shader)
{
//	const ShaderMan *sm = this->shader_to_draw;
	if (!different_shader)
		throw std::runtime_error("no shader to draw");
	for (GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].draw(different_shader, *this);
}

void
Model::make_instances(const int n_instances, InstanceINIT flag)
{

	glm::quat default_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
	glm::vec3 default_scale(0.5f);
	
	if (flag == INIT_random) {
		this->instances.translations.resize(n_instances);
		this->instances.rotations.resize(n_instances);
		this->instances.scales.resize(n_instances);
		
		std::random_device rd;
		std::minstd_rand el(rd());

		for (int i = 0; i < n_instances; i++) {
			this->instances.translations[i] = glm::vec3(el(), el(), el());
			this->instances.rotations[i] = default_rotation;
			this->instances.scales[i] = default_scale;
		}
	} else {
		//for square example
		//take 1000 as example
		int rows = n_instances; //31
		int cols = n_instances; //32, 31*32 = 992
		int count = 0;

		this->instances.translations.resize(rows * cols);
		this->instances.rotations.resize(rows * cols);
		this->instances.scales.resize(rows * cols);
		
		for (int i = 0; i < rows; i++) {
			for( int j = 0;  j < cols; j++) {
				this->instances.translations[count] = glm::vec3((float)i, 0.0f, (float)j);
				this->instances.rotations[count] = default_rotation;
				this->instances.scales[count] = default_scale;
				count += 1;
			}
		}
	}
	//you don't know whether the meshes is uploaded to GPU
}

void Model::pushIntances2GPU()
{
	if (this->instanceVBO)
		glDeleteBuffers(1, &this->instanceVBO);
	
	std::vector<glm::vec3>& trs = this->instances.translations;
	std::vector<glm::quat>& rts = this->instances.rotations;
	std::vector<glm::vec3>& scs = this->instances.scales;
	size_t ninstances = this->instances.translations.size();
	
	std::vector<glm::mat4> instance_mats(ninstances);
	for (size_t i = 0; i < instance_mats.size(); i++)
		instance_mats[i] = glm::translate(trs[i]) * glm::mat4_cast(rts[i]) * glm::scale(scs[i]);

	glGenBuffers(1, &this->instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instance_mats.size(), &instance_mats[0], GL_STATIC_DRAW);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);

	size_t vec4_size = sizeof(glm::vec4);
	
	for (size_t i = 0; i < meshes.size(); i++) {
		glBindVertexArray(meshes[i].VAO);
//		glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);

		GLuint va = this->get_layout_count();
		glEnableVertexAttribArray(va);
		glVertexAttribPointer(va, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
		glEnableVertexAttribArray(va+1);
		glVertexAttribPointer(va+1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)vec4_size);
		glEnableVertexAttribArray(va+2);
		glVertexAttribPointer(va+2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2*vec4_size));
		glEnableVertexAttribArray(va+3);
		glVertexAttribPointer(va+3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3*vec4_size));

		glVertexAttribDivisor(va, 1);
		glVertexAttribDivisor(va+1, 1);
		glVertexAttribDivisor(va+2, 1);
		glVertexAttribDivisor(va+3, 1);

		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
		


//cpu based instancing
CubeModel::CubeModel(const glm::vec3 translation, const glm::vec3 scale, const glm::quat rotation)
{
	this->instanceVBO = 0;
	this->meshes.push_back(Mesh(CUBEVERTS, CUBENORMS, CUBETEXS, 36));
	glm::quat default_rotation = glm::quat(glm::vec3(0.0f));
	
	glm::mat3 position_mat = glm::mat3_cast(rotation) * glm::mat3(glm::scale(scale));
	glm::mat3 normal_mat   = glm::transpose(glm::inverse(position_mat));
	
	std::vector<glm::vec3>& positions = this->meshes[0].vertices.Positions;
	std::vector<glm::vec3>& normals   = this->meshes[0].vertices.Normals;
	//see if we need to do anything
	if (translation == glm::vec3(0.0f) && scale == glm::vec3(1.0f) && rotation == default_rotation)
		return;
	
	for (size_t i = 0; i < positions.size(); i++) {
		positions[i] = translation + position_mat * positions[i];
		if (!normals.empty())
			normals[i] = normal_mat * normals[i];
	}

}


