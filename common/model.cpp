#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>
#include <queue>
#include <string>

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

sprt_tex2d_t texture_types_supported[TEX_NTexType] = {
	{aiTextureType_AMBIENT, TEX_Ambient},
	{aiTextureType_DIFFUSE, TEX_Diffuse},
	{aiTextureType_NORMALS, TEX_Normal},
	{aiTextureType_SPECULAR, TEX_Specular}
};

GLint
loadTexture2GPU(const std::string fname)
{
	GLuint tid;

	glGenTextures(1, &tid);
	glBindTexture(GL_TEXTURE_2D, tid);
	cv::Mat img = cv::imread(fname, CV_LOAD_IMAGE_COLOR);
	if (!img.data) {
		std::cerr << "not valid image" << std::endl;
		return -1;
	}
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

Model::Model(const std::string& file, int param)
{
	//setup constant
	this->instanceVBO = 0;
	
	aiScene *scene = this->readModel(file);
	this->root_path = file.substr(0, file.find_last_of('/'));

	//definitly load the mesh
	for (uint i = 0; i < scene->mNumMeshes; i++) {
		this->meshes.push_back(Mesh(scene, scene->mMeshes[i]));
	}
	//load the bone, TODO; make it in a wrapper
	if (param & LOAD_BONE) {
		//load bone has to be walk through the nodes, because only mesh
		//has bone data. Then we can use the mesh to load bone weight
		this->loadBone(scene, scene->mRootNode);
		for (uint i = 0; i < scene->mNumMeshes; i++)
			this->meshes[i].loadBoneWeights(scene->mMeshes[i], *this);
		root_bone = this->findRootBone(scene, scene->mRootNode);
		assert(root_bone ==
		       processBoneNode(scene,
				       scene->mRootNode->FindNode(root_bone->name().c_str())));

	}
	//find the root bone. Then We can load the heirachy
	//then finally we can push the data to the GPU
	for (GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].pushMesh2GPU();

	//setup the meshlayout
	this->n_mesh_layouts = 3;
	//if load texture
	if (!(param & NO_TEXTURE))
		this->loadMaterials(scene);

	if (param & LOAD_ANIM && scene->HasAnimations())
		this->loadAnimations(scene);
	//TODO, updload verything to GPU should be here

	//TODO cleanup scene
	delete scene;
	//TODO Delete maybe we should delete the CPU data
}


Model::Model()
{
}

Model::~Model()
{
	if (this->instanceVBO)
		glDeleteBuffers(1, &this->instanceVBO);
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



Bone*
Model::processBoneNode(const aiScene *scene, const aiNode *node)
{
	//find the bone and parent_bone. if parent_bone is not set. then This is
	//the root bone

	std::string node_name = node->mName.data;
	std::string parent_name = node->mParent->mName.data;
	Bone &bone = this->bones[node_name];
	auto parent_itr = this->bones.find(parent_name);
	bone.children.clear();
	
	glm::mat4 parent_transform;	
	glm::mat4 local_transform;
	if (parent_itr != this->bones.end()) 
		bone.parent = &parent_itr->second;
	else
		bone.parent = NULL;
	
	bone.setModelMat(aiMat2glmMat(node->mTransformation));
	bone.setStackedTransformMat();
	
	for(GLuint i = 0; i < node->mNumChildren; i++)
	{
		Bone *child_node;
		if ((child_node = this->processBoneNode(scene, node->mChildren[i])) != NULL) {
			bone.children.push_back(child_node);
		}
	}
	return &bone;
}


const Bone*
Model::findRootBone(const aiScene *scene, const aiNode *node)
{
	//use the BFS searching
	std::queue<aiNode *> node_queue;
	node_queue.push(scene->mRootNode);
	while(!node_queue.empty()) {
		aiNode *current = node_queue.front();
		node_queue.pop();
		std::string potential_bone = current->mName.C_Str();
		const auto itr = this->bones.find(potential_bone);
		if (itr != this->bones.end()) {
			return &itr->second;
		}
		for (uint i = 0; i < current->mNumChildren; i++) {
			node_queue.push(current->mChildren[i]);
		}

	}
	return NULL;
}


int
Model::loadBone(const aiScene *scene, const aiNode *node)
{
	int count = 0;
	auto& all_bones = this->bones;
	for(GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		for (GLuint j = 0; j < mesh->mNumBones; j++) {
			std::string bone_name = mesh->mBones[j]->mName.data;
			if (all_bones.find(bone_name) == all_bones.end()) {
				Bone local_bone(bone_name);
				local_bone.setInd(all_bones.size());
				all_bones[bone_name] = local_bone;
				count +=1;
			}
		}
	}
	for (GLuint i = 0; i < node->mNumChildren; i++) {
		count += this->loadBone(scene, node->mChildren[i]);
	}
	//total number of bones
	return count;
}


void
Model::draw(const ShaderMan *different_shader)
{
//	const ShaderMan *sm = this->shader_to_draw;
	if (!different_shader)
		different_shader = this->shader_to_draw;
	for (GLuint i = 0; i < this->meshes.size(); i++)
		this->meshes[i].draw(different_shader, *this);
}

void
Model::makeInstances(const int n_instances, const InstanceINIT flag,
		      //additional arguments, may get ignored
			    const glm::vec3 default_scale,
			    const glm::vec3 default_translation,
			    const glm::quat default_rotation)
{
	//in current case, ignore translation
	(void)default_translation;
	
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
	//also, we need a way to upload bone weights to instances
}

void
Model::appendInstance(const glm::vec3 translation,
		      const glm::vec3 scale,
		      const glm::quat rotation)
{
	this->instances.translations.push_back(translation);
	this->instances.scales.push_back(scale);
	this->instances.rotations.push_back(rotation);
}


void
Model::pushIntances2GPU()
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

		GLuint va = this->getLayoutCount();
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


void
Model::push2GPU(int param)
{
	//get the proper texture 
	this->n_mesh_layouts = 1;
	if (param & Mesh::LOAD_NORMAL)
		this->n_mesh_layouts += 1;
	if (param & Mesh::LOAD_TEX)
		this->n_mesh_layouts += 1;
		
	for (unsigned int i = 0; i < this->meshes.size(); i++)
		this->meshes[i].pushMesh2GPU(param);
		
	//We can do it here or 
	if (this->instances.translations.size() > 0 && instanceVBO == 0)
		this->pushIntances2GPU();
}

int
Model::loadMaterials(const aiScene *scene)
{
	std::map<std::string, GLuint> textures_cache;

	this->Materials.resize(scene->mNumMaterials);
	for (GLuint i = 0; i < scene->mNumMaterials; i++) {
		std::cerr << "material indx: " << i << std::endl;
		aiMaterial *mat = scene->mMaterials[i];
		aiString path;
		Material material;
		GLuint gpu_handle;
		for (GLuint j = 0; j < TEX_NTexType; j++) {
			if (mat->GetTextureCount(texture_types_supported[j].aiTextype) > 0) {
				mat->GetTexture(texture_types_supported[j].aiTextype, 0, &path);
				std::string full_path = this->root_path + "/" + std::string(path.C_Str());
//				std::cerr << full_path << std::endl;
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
	return scene->mNumMaterials;
}


int
Model::loadAnimations(const aiScene* scene)
{
	std::cout << "I am here, with " << scene->mNumAnimations << " animations" << std::endl;
	this->animations.resize(scene->mNumAnimations);
	for (uint i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation *anim = scene->mAnimations[i];		
//		this->animations[i].keyframes.resize(this->bones.size());
		//I am so confused... only one mTickPerSecond???
		size_t total_frames = anim->mTicksPerSecond * anim->mDuration;
		for (int k = 0; k < anim->mNumChannels; k++) {
//			std::cout << "here"  << std::endl;
			aiNodeAnim *bone_anim = anim->mChannels[k];
			//find the bone
			std::string name = bone_anim->mNodeName.C_Str();
			std::cout << bone_anim->mNumPositionKeys << " translations, ";
//			int ind = this->bones[name].getInd();
			std::cout << bone_anim->mNumRotationKeys << " rotations, and ";
			std::cout << bone_anim->mNumScalingKeys << " scales\n";
//			(void)bone_anim->mNumPositionKeys;
//			(void)bone_anim->mNumRotationKeys;
//			(void)bone_anim->mNumScalingKeys;
		}
	}
	
}


