#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <map>
#include <string>


#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


//asset loading library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "../include/model.hpp"


//I don't know if a show share the same shaders
void
Scene::draw(void)
{
	std::for_each(this->meshes.begin(), this->meshes.end(),
		      [](std::pair<MeshNode, glm::vec3> m&) {m.first.draw(m.second);})
}

//here is my question, if one node shares mesh with other nodes, it should have the same mesh coordinates with other nodes.


//this is actually a loading scene function, since this is what we can get from a obj file,
//we assume it only have one 
void
Scene::loadModel(const char *path)
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

	unsigned int node_id = 0;
	this->children.push_back(0);
	this->nodes.push_back(modelnode_t(model_name, ModelNode(0, -1)));
        ModelNode *model = &this->nodes[0].second;

	std::cout << "loading a model" << std::endl;
	//we have to load the mode in the old fasion way, because we don't know whether it works from
	model->processNode(scene->mRootNode, scene, this, directory, aiMatrix4x4());
	std::cout << "done loading a model" << std::endl;
	
	//here is probably where I fucked up, I should only update those numbers from here
	this->loaded_mesh += scene->mNumMeshes;
	this->loaded_material += scene->mNumMaterials;
	this->loaded_nodes = this->nodes.size();
	//after this, we need to setup mesh through the list
}


void
Scene::append_mesh(aiMesh* mesh)
{
	MeshNode m;

	//processing vertices
	for (size_t i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		//here we has the problem, normal can be textures as well, but
		//most obj files write them directly in the file
		//it should have a normal.
		vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//TODO: assimp allows max 8 texcoords system, but we only use the first one.
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
	//load 
	if (mesh->mMaterialIndex >= 0)
	{
		m.materials
	}

	
}

//this function should be like this.
void
ModelNode::processNode(aiNode *ainode, const aiScene *aiscene, Scene *scene,
		       //data terms
		       const std::string& dir, const aiMatrix4x4& transform)
{
	aiMatrix4x4& node_transform = ainode->mTransformation;
	//loading its own meshes
	for (size_t i = 0; i < ainode->mNumMeshes; i++) {

		if (!scene->getMesh(ainode->mMeshes[i]))
			scene->append_mesh(aiscene->mMeshes[ainode->mMeshes[i]]);
		
		//then we need to append the trasformatin information to the mesh list
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
		this->child_nodes.push_back(node_id);		
		ModelNode *node = &scene->nodes[node_id].second;

		//finally, processing the children node
		node->processNode(child_node, aiscene, this, dir, node_transform);
	}
}



