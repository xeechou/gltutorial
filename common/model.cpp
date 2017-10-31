#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <queue>
#include <string>
#include <set>
#include <cmath>

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

#include <shaderman.h>
#include <utils.h>
#include <model.hpp>
#include <data.hpp>

/**
 * @brief describes the order of properties
 */
enum PROPERTYorder {
	mesh = 0, //load mesh first
	instancing = 1, //instancing has to draw and push after
	material = 2,
};

static int
getPropertyOrder(const std::string& name)
{
	if (name == "mesh")
		return PROPERTYorder::mesh;
	else if (name == "material")
		return PROPERTYorder::material;
	else if (name == "instancing")
		return PROPERTYorder::instancing;
	return -1;
}


//Model::Model(const std::string& file, int param)
//{
//	this->drawproperty = nullptr;
//}


Model::Model()
{
	this->drawproperty = nullptr;
}

Model::~Model()
{
}

aiScene*
Model::readModel(const std::string& file)
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
		throw std::runtime_error(std::string("ERROR::ASSIMP") + import.GetErrorString());\
	//this code
	aiScene *orph_scene = import.GetOrphanedScene();
	return orph_scene;
}





const std::string
Model::getRootPath(void) const
{
	return this->root_path;
}

bool
Model::addProperty(const std::string &name, std::shared_ptr<OBJproperty> data)
{
	int order = getPropertyOrder(name);
	
	if (this->properties.find(order) != this->properties.end() || order < 0)
		return false;
	this->properties[order] = std::make_pair(name, data);
	data->bindModel(this);
	if (data->isdrawPoint()) {
		assert(this->drawproperty == nullptr);
		this->drawproperty = data;
	}
	return true;
}

OBJproperty*
Model::searchProperty(const std::string name) const
{
	int order = getPropertyOrder(name);
	if (this->properties.find(order) != this->properties.end())
		return this->properties.at(order).second.get();
	return NULL;
}

void
Model::drawProperty(const ShaderMan *differentShader)
{
	const ShaderMan *origin_shader = this->shader_to_draw;
	if (differentShader)
		this->bindShader(differentShader);
	this->drawproperty->draw(msg_t());
	this->bindShader(origin_shader);
}

void
Model::load(const std::string &file)
{
	this->root_path = file.substr(0, file.find_last_of('/'));	
	aiScene *scene = this->readModel(file);
	
	int layout_start = 0;
	for (auto it = this->properties.begin(); it != this->properties.end(); it++) {
		auto pair = it->second;
//		std::cerr << pair.first << std::endl;
		//in this case, every oproperty has layout
		pair.second->alloc_shader_layout(layout_start);
		layout_start = pair.second->getLayoutsEnd();
		pair.second->load(scene);
	}
	delete scene;
}



void
Model::push2GPU(void)
{
	for (auto it = this->properties.begin(); it != this->properties.end(); it++) {
		auto pair = it->second;
		pair.second->push2GPU();
	}
}
