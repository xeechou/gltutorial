#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <map>
#include <tuple>
#include <string>


#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


//asset loading library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


//OpenCV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "../include/model.hpp"

void aimat2glmmat(const aiMatrix4x4& src, glm::mat4& dst)
{
	//first col
	dst[0][0] = src.a1; dst[1][0] = src.a2;
	dst[2][0] = src.a3; dst[3][0] = src.a4;
	//second col
	dst[0][1] = src.b1; dst[1][1] = src.b2;
	dst[2][1] = src.b3; dst[3][1] = src.b4;
	//third col
	dst[0][2] = src.c1; dst[1][2] = src.c2;
	dst[2][2] = src.c3; dst[2][3] = src.c4;
	//last col
	dst[0][3] = src.d1; dst[1][3] = src.d2;
	dst[2][3] = src.d3; dst[3][3] = src.d4;
}
void aimat2array(const aiMatrix4x4& src, float *start)
{
//four rows
	//first col
	*(start+0) = src.a1; *(start+1) = src.a2;
	*(start+2) = src.a3; *(start+3) = src.a4;
	//second col
	*(start+4) = src.b1; *(start+5) = src.b2;
	*(start+6) = src.b3; *(start+7) = src.b4;
	//third col
	*(start+8) = src.c1; *(start+9) = src.c2;
	*(start+10) = src.c3; *(start+11) = src.c4;
	//last col
	*(start+12) = src.d1; *(start+13) = src.d2;
	*(start+14) = src.d3; *(start+15) = src.d4;
}

static GLuint
texture_from_file(const char *path, const std::string& directory)
{
	std::string fname(path);
	fname = directory + '/' + fname;
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

//shader should provide map for this

void
MeshNode::drawInstanced(GLuint program, Scene *scene)
{
	glUseProgram(program);

	for (size_t i = 0; i < this->materials.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		struct Tex& texture = scene->materials[this->materials[i]].second;
		glBindTexture(GL_TEXTURE_2D, texture.id);
		if (texture.type == TEX_DIFFUSE_MAP)
			glUniform1i(glGetUniformLocation(program, "diffuse"), i);
		else if (texture.type == TEX_SPECULAR_MAP)
			glUniform1i(glGetUniformLocation(program, "specular"), i);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_BUFFER, this->instanceTex);
	//The actual data is in 
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, scene->instances[this->model_id]);
	//every shader should have 
	glUniform1i(glGetUniformLocation(program, "instanceTex") , 0);

	glBindVertexArray(this->VAO);
	glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, this->transforms.size());
	glBindVertexArray(0);
}
void
MeshNode::draw(GLuint program, Scene *scene)
{
	glUseProgram(program);

	for (size_t i = 0; i < this->materials.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		struct Tex& texture = scene->materials[this->materials[i]].second;
		glBindTexture(GL_TEXTURE_2D, texture.id);
		if (texture.type == TEX_DIFFUSE_MAP)
			glUniform1i(glGetUniformLocation(program, "diffuse"), i);
		else if (texture.type == TEX_SPECULAR_MAP)
			glUniform1i(glGetUniformLocation(program, "specular"), i);
	}
	glActiveTexture(GL_TEXTURE0);
//	glBindTexture(GL_TEXTURE_BUFFER, this->instanceTex);
	//The actual data is in 
//	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, scene->instances[this->model_id]);
	//every shader should have 
//	glUniform1i(glGetUniformLocation(program, "instanceTex") , 0);

	glBindVertexArray(this->VAO);
	glDrawElementsInstanced(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0, this->transforms.size());
	glBindVertexArray(0);
}


void
MeshNode::setupMesh_forshader(GLuint program, Scene *scene)
{
	//Using this code means we need to have the same input layout for shaders

	glUseProgram(program);
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);
	//glGenBuffers(1, &this->instanceTBO);
	glGenTextures(1, &this->instanceTex);
	//float model_mats[this->transforms.size()][16];

	//this is actually not a good idea, we can only draw static scenes

	//TODO ignore the current node transformation though, add it later on at animation
	//for (int i = 0; i < this->transforms.size(); i++)
	//	aimat2array(this->transforms[i].second, model_mats[i]);
	//glBindBuffer(GL_TEXTURE_BUFFER, this->instanceTBO);
	//glBufferData(GL_TEXTURE_BUFFER, sizeof(model_mats), model_mats, GL_STATIC_DRAW);

	glBindVertexArray(this->VAO);
	//the stride is sizeof(Vertex), a size of a structure, this is new	
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(struct Vertex),
		     &this->vertices[0], GL_STATIC_DRAW);
	//setting indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
		     &this->indices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	//Vertex positons for layouts
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			      (GLvoid *)0);
	//Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			      (GLvoid *)offsetof(struct Vertex, Normal));
	//Vertex Texture Coords?
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
			      (GLvoid *)offsetof(struct Vertex, TexCoords));

	//for instancing, we want to use texture to do so
	glBindVertexArray(0);

	
}

//I don't know if a show share the same shaders
void
Scene::draw(GLuint program)
{
	//TODO The skybox code should be added here
	//I need a program
	for (size_t i = 0; i < this->children.size(); i++) {
		model_ind_t& child = this->children[i];
		std::vector<unsigned int>& mesh = std::get<0>(child);
		for (size_t j = 0; j < meshes.size(); j++) {
			auto it = this->instances.find(std::get<1>(child));
			if ( it != this->instances.end() )
				this->meshes[mesh[j]].drawInstanced(program, this);
			else
				this->meshes[mesh[j]].draw(program, this);
		}
	}
}


GLuint
Scene::setupModelInstance(const std::vector<glm::mat4>& transforms)
{
	GLuint instanceTBO;
	glGenBuffers(1, &instanceTBO);
//	glGenTextures(1, &instanceTex);

	//this is actually not a good idea, we can only draw static scenes
	glBindBuffer(GL_TEXTURE_BUFFER, instanceTBO);
	glBufferData(GL_TEXTURE_BUFFER, sizeof(glm::mat4) * transforms.size(), &transforms[0], GL_STATIC_DRAW);
	return instanceTBO;
}

//this is actually a loading scene function, since this is what we can get from a obj file,
//we assume it only have one 
void
Scene::loadModel(const char *path, const std::vector<glm::mat4>& instances_transform)
{
	Assimp::Importer import;
	//OpenGL the image coordinates starts from buttum left. OpenCV starts
	//from topleft. So we need to flip UV coordinates
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	std::string directory;
	std::string model_name;
	{
		std::string path_str(path);
		directory = path_str.substr(0, path_str.find_last_of('/'));
		model_name = path_str.substr(path_str.find_last_of('/'), path_str.find_last_of('.'));
	}


	//adding new model to the scene, node_id is the root node for the model
	unsigned int node_id = this->nodes.size();
	model_ind_t child(std::vector<unsigned int>(), node_id);
	this->children.push_back(child);
	//setting up instance transforms
	if (instances_transform.size())
		this->instances[node_id] = this->setupModelInstance(instances_transform);
	
	//setting up the real nodes 
	//-1 is the scene id.
	this->nodes.push_back(modelnode_t(model_name, ModelNode(node_id, -1)));
        ModelNode *model = &this->nodes[0].second;

	std::cout << "loading a model" << std::endl;
	//First step, loading models.
	model->processNode(scene->mRootNode, scene, this, node_id, directory, aiMatrix4x4());
	std::cout << "done loading a model" << std::endl;
	//multiple nodes can referes to the same mesh, in the way they can support instancing,
	//but fuck that, this could make things becomes very nasty, on node should use one mesh

	//second, setting up mesh for each node
	for (size_t i = this->loaded_mesh; i < this->meshes.size(); i++)
		this->meshes[i].setupMesh_forshader(0, this);

	for (size_t i = 0; i < scene->mNumMaterials; i++) {

		const aiMaterial *material = scene->mMaterials[i];
		this->processingMaterial(material, directory);		
	}
	
	//load the materials
	//here is probably where I fucked up, I should only update those numbers from here
	this->loaded_mesh += scene->mNumMeshes;
	this->loaded_material += scene->mNumMaterials * NUM_MATERIAL_TYPE;
	this->loaded_nodes = this->nodes.size();
	//after this, we need to setup mesh through the list
	//TODO free scene
}


void
Scene::append_mesh(aiMesh* mesh, const aiScene *scene, unsigned int model_id)
{
	MeshNode m;
	m.model_id = model_id;
	//processing vertices
	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		//it should have a normal.
		vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//FIXME: assimp allows max 8 texcoords system, but we only use the first one.
		if (mesh->HasTextureCoords(0))
			vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		//
		m.vertices.push_back(vertex);
	}
	//every triangle is a face
	for (GLuint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		//here we could do a optimization, since a triangle is always 3...
		for (size_t j = 0; j < face.mNumIndices; j++)
			m.indices.push_back(face.mIndices[j]);
	}
	
	//draw mesh with materials
	if (mesh->mMaterialIndex >= 0) {
//		aiString aistr;
//		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		m.materials.clear();
		m.materials.push_back(this->getMaterialID(mesh->mMaterialIndex, TEX_DIFFUSE_MAP));
		m.materials.push_back(this->getMaterialID(mesh->mMaterialIndex, TEX_SPECULAR_MAP));
//		material->GetTexture(aiTextureType_DIFFUSE, 0, &aistr);
//		material->GetTexture(aiTextureType_SPECULAR, 0, &aistr);
//		m.material = mesh->mMaterialIndex;
	}
	//last step, append mesh
	this->meshes.push_back(m);
}

void
Scene::processingMaterial(const aiMaterial *material, const std::string& directory)
{
	//initialize your the data, here we have to make id = -1 
	std::pair<std::string, struct Tex> init_data(std::string(), (struct Tex){-1, TEX_UNKNOWN_MAP});
	std::vector<std::pair<std::string, struct Tex> >mats(NUM_MATERIAL_TYPE, init_data);

	aiString aistr;
	for (size_t j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++) {
		if (material->GetTexture(aiTextureType_DIFFUSE, j, &aistr)) {
			GLuint tid = texture_from_file(aistr.C_Str(), directory);
			mats[j] = std::pair<std::string, struct Tex>(std::string(aistr.C_Str()), (struct Tex){tid, TEX_DIFFUSE_MAP});
			break;
		}
	}
	for (size_t j = 0; j < material->GetTextureCount(aiTextureType_SPECULAR); j++) {
		if (material->GetTexture(aiTextureType_SPECULAR, j, &aistr)) {
			GLuint tid = texture_from_file(aistr.C_Str(), directory);
			mats[j] = std::pair<std::string, struct Tex>(std::string(aistr.C_Str()), (struct Tex){tid, TEX_SPECULAR_MAP});
			break;
		}
		
	}
	this->materials.insert(this->materials.end(), mats.begin(), mats.end());
}

//how to do instancing, this is not a right way.
void
ModelNode::processNode(aiNode *ainode, const aiScene *aiscene, Scene *scene,
		       //data terms
		       unsigned int model_id,
		       const std::string& dir, const aiMatrix4x4& transform)
{
	aiMatrix4x4& node_transform = ainode->mTransformation;
	//loading its own meshes
	for (size_t i = 0; i < ainode->mNumMeshes; i++) {

		if (!scene->getMesh(ainode->mMeshes[i]))
			scene->append_mesh(aiscene->mMeshes[ainode->mMeshes[i]], aiscene, model_id);
		
		//then we need to append the trasformatin information to the mesh list
		//maybe I should keep this, perhaps the we node uses the same mesh?
		scene->getMesh(ainode->mMeshes[i])->transforms.push_back(
			std::pair<int, aiMatrix4x4>(this->node_id, node_transform) );
	}
	//and processing children
	for (size_t i = 0; i < ainode->mNumChildren; i++) {
		//in this way, we should be able to
		aiNode *child_node = ainode->mChildren[i];
		std::string nodeName(child_node->mName.C_Str());
		size_t node_id = scene->nodes.size();
		scene->nodes.push_back(modelnode_t(nodeName,
						   ModelNode(node_id, this->node_id)));
		std::cout << nodeName << std::endl;
		this->child_nodes.push_back(node_id);		
		ModelNode *node = &scene->nodes[node_id].second;

		//processing children nodes, rightnow, we just ignore this node_transform infomation
		node->processNode(child_node, aiscene, scene,
				  model_id, dir, node_transform);
	}
}



