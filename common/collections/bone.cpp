#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <array>
#include <string>
#include <tuple>
#include <algorithm>
#include <random>


#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <tree.hpp>
#include <types.hpp>
#include <property.hpp>
#include <model.hpp>
#include <shaderman.h>
#include <data.hpp>
#include <collections/bone.hpp>


static cv::Mat_<cv::Vec2f>
assembleWeights(cv::Mat& weights, cv::Mat& indx)
{
	//we need to return a two channel matrix
	//even with assemble weights. we still could not get the
	cv::Mat_<cv::Vec2f> boneWeights(weights.rows, weights.cols);
	cv::Mat tmprow(1,weights.cols, weights.type());
	for (int i = 0; i < boneWeights.rows; i++) {
		cv::normalize(weights.row(i), tmprow, 1,0,cv::NORM_L1);
		tmprow.copyTo(weights.row(i));
		for( int j = 0;  j < boneWeights.cols; j++)
			boneWeights(i,j) = cv::Vec2f((float)indx.at<int>(i,j), weights.at<float>(i,j));
	}
	//I can use
	return boneWeights;
}


Bone::Bone(int indx, const std::string id, const glm::mat4& m) :
	TreeNode(id), _offsetMat(m), _index(indx)
{
	//we are setting up the stacked transform later
}

Bone::Bone(const Bone& bone) : TreeNode(bone.id, bone._offsetMat)
{
	//we cannot assume it has other code
	_offsetMat = bone._offsetMat;
	_index = bone._index;
	id = bone.id;
}

Bone::Bone(const Bone&& bone)
{
	this->id = bone.id;
	this->_offsetMat = bone._offsetMat;
	this->_index = bone._index;
}

//there is nothing else you can do, but give this
Bone::Bone(void) : TreeNode("", glm::mat4(1.0f))
{

}
Bone::~Bone()
{
}

void
Bone::setStackedTransformMat()
{
	TreeNode::setStackedTransformMat();
	this->_invTransform = glm::inverse(this->_cascade_transform);
}


Skeleton::Skeleton(uint weights, const std::string& uniform_name)
{
	this->shader_layouts.second=weights;
	this->uniform_bone = uniform_name;
}

Skeleton::~Skeleton()
{
	for (uint i = 0; i < this->gpu_handles.size(); i++)
		glDeleteBuffers(1, &gpu_handles[i]);
}

bool
Skeleton::load(const aiScene *scene)
{
	mb_weights.resize(scene->mNumMeshes);
	mb_indices.resize(scene->mNumMeshes);
	for (uint i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[i];
		//initialize this first
		this->mb_weights[i] = cv::Mat_<float>::zeros(mesh->mNumVertices, this->shader_layouts.second);
		this->mb_indices[i] = cv::Mat_<int>::zeros(mesh->mNumVertices, this->shader_layouts.second);

		for (uint j = 0; j < mesh->mNumBones; j++) {
			const std::string bone_name = std::string(mesh->mBones[j]->mName.C_Str());
			aiBone *aibone = mesh->mBones[j];
			if (this->bones.find(bone_name) == this->bones.end()) {
//				std::cerr << bone_name << std::endl;
				this->bones.insert(std::make_pair(bone_name,
								  Bone(this->bones.size(), bone_name, aiMat2glmMat(aibone->mOffsetMatrix))
							   ));
			}
			this->loadBoneWeights(scene, i, j);
		}
	}
	//all right, since I loaded up all the meshes
	this->buildHierachy(scene, findRootBone(scene));
//	std::cerr << this->root_bone->layout() << std::endl;
	this->cascade_transforms.resize(this->bones.size());
	//a fake transform, which does nothing at all
	std::fill(this->cascade_transforms.begin(), this->cascade_transforms.end(), glm::mat4(1.0f));
	//TODO now we do the intial transform
	return true;
}

/*
static void debug_bw(cv::Mat& debug)
{
	int c;
	std::cerr << debug.channels() << std::endl;
	for (int i = 0; i < debug.rows; i++) {
		std::cerr << cv::format(debug.row(i), cv::Formatter::FMT_NUMPY) << std::endl;
		std::cin >> c;
	}
}

static void assert_weight(cv::Mat& weights)
{
	for (int i = 0; i < weights.rows; i++) {
		float sum = 0;
		for (int j = 0; j < weights.cols; j++)
			sum += weights.at<float>(i,j);
		if (sum != 1.0f)
			std::cout << weights.row(i) << '\t';
	}

}
*/

bool
Skeleton::push2GPU()
{
	this->gpu_handles.resize(this->mb_weights.size());
	uint first_layout = this->shader_layouts.first;
	uint layout_ends  = this->shader_layouts.first + this->shader_layouts.second;
	Mesh1* mesh_handle = (Mesh1*)this->model->searchProperty("mesh");
	size_t vec2_size = sizeof(cv::Vec2f);
	for (uint i = 0; i < this->mb_weights.size(); i++) {
		//NOTE after assembled weight, we still have float error problems
		cv::Mat_<cv::Vec2f> bw = assembleWeights(this->mb_weights[i], this->mb_indices[i]);
//		assert_weight(this->mb_weights[i]);
//		debug_bw(bw);
		mesh_handle->activeIthMesh(i);
		glGenBuffers(1, &this->gpu_handles[i]);
		glBindBuffer(GL_ARRAY_BUFFER, this->gpu_handles[i]);
		glBufferData(GL_ARRAY_BUFFER, bw.rows * bw.step[0], (void *)bw.data, GL_STATIC_DRAW);

		for (uint l = first_layout; l < layout_ends; l++) {
			glEnableVertexAttribArray(l);
			glVertexAttribPointer(l, 2, GL_FLOAT, GL_FALSE,
					      sizeof(cv::Vec2f),
					      (void *)((l-first_layout) * vec2_size));
		}
		glBindVertexArray(0);
		//useless
		this->mb_weights.clear();
		this->mb_indices.clear();
	}
//	const ShaderMan *sm = this->getBindedShader();


	return true;
}

/*
bool
Skeleton::uploadUniform(const ShaderMan *sm)
{
	//for this one, we actually need to update the uniforms in the draw call.
//	GLuint prog = sm->getPid();
//	glUseProgram(prog);
	return true;
}
*/

void
Skeleton::loadBoneWeights(const aiScene *s, int meshi, int bonej)
{
	cv::Mat_<float>& weights = this->mb_weights[meshi];
	cv::Mat_<int>& indices = this->mb_indices[meshi];
	aiMesh *mesh = s->mMeshes[meshi];
	aiBone *bone = mesh->mBones[bonej];
	std::string bone_name = std::string(bone->mName.C_Str());
	//find the correct id of that bone
	auto it = this->bones.find(bone_name);
	int j  = it->second.getInd();
	for (uint i = 0; i < bone->mNumWeights; i++) {
		uint vid = bone->mWeights[i].mVertexId;
		float w  = bone->mWeights[i].mWeight;
//		std::cerr << "vertex " << vid << ", weight " << w << std::endl;
		int k;
		for (k = 0; k < this->shader_layouts.second; k++) {
			if (weights(bone->mWeights[i].mVertexId, k) == 0) {
				weights(vid, k) = w;
				indices(vid, k) = j;
				break;
			}
		}
		//it should never get here
		assert(k != this->shader_layouts.second);
	}
}


aiNode *
Skeleton::findRootBone(const aiScene *scene) const
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
			return current;
		}
		for (uint i = 0; i < current->mNumChildren; i++) {
			node_queue.push(current->mChildren[i]);
		}
	}
	return NULL;
}

void
Skeleton::buildHierachy(const aiScene *scene, const aiNode *root_node)
{
	std::queue<const aiNode *> fifo_nodes;
	this->root_bone = &this->bones[root_node->mName.data];
	fifo_nodes.push(root_node);
	while (!fifo_nodes.empty()) {
		const aiNode *node = fifo_nodes.front();
		fifo_nodes.pop();
		std::string name = node->mName.data;
		std::string parent_name = node->mParent->mName.data;
		Bone& thisbone = bones[name];
		thisbone._model_mat = aiMat2glmMat(node->mTransformation);
		//setup the parent first
		if (this->bones.find(parent_name) != this->bones.end()) {
			thisbone.parent = &this->bones[parent_name];
			thisbone._cascade_transform *= thisbone.parent->_cascade_transform;
		} else {
			thisbone.parent = NULL;
			thisbone._cascade_transform = thisbone._model_mat;
		}
		//now the children
		for (uint i = 0; i < node->mNumChildren; i++) {
			std::string childname = node->mChildren[i]->mName.data;
			if (this->bones.find(childname) != this->bones.end()) {
				thisbone.children.push_back(&this->bones[childname]);
				fifo_nodes.push(node->mChildren[i]);
			}
		}
	}
}


void
Skeleton::draw(const msg_t msg)
{
	//if animation is applied here, we should update the local bone
	//transformations. But before that, let's just load the uniforms
	(void)msg;
	const ShaderMan *sm = this->getBindedShader();
	sm->useProgram();
	//you should flash the boneweights
	glUniformMatrix4fv(sm->getUniform(uniform_bone), this->bones.size(), GL_FALSE, (GLfloat *)&this->cascade_transforms[0]);


}
