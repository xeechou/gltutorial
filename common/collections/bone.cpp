#include <iostream>
#include <fstream>
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
	return boneWeights;
}


Bone::Bone(const std::string id, const glm::mat4& m) :
	TreeNode(id), _offsetMat(m)
{
	//we are setting up the stacked transform later
}

Bone::Bone(const Bone& bone)
{
	this->id = bone.id;
	this->_offsetMat = bone._offsetMat;
}

Bone::Bone(const Bone&& bone)
{
	this->id = bone.id;
	this->_offsetMat = bone._offsetMat;
//	this->_index = bone._index;
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
			if (this->bone_names.find(bone_name) == this->bone_names.end()) {
				this->bone_names[bone_name] = this->bones.size();
				this->bones.emplace_back(Bone(bone_name, aiMat2glmMat(aibone->mOffsetMatrix)));

			}
			this->loadBoneWeights(scene, i, j);
		}
	}
	//all right, since I loaded up all the meshes
	this->buildHierachy(scene, findRootBone(scene));
//	std::cerr << this->root_bone->layout() << std::endl;
	this->cascade_transforms.resize(this->bones.size());
	//debug, if the layout is correct
	std::cout << this->root_bone->layout() << std::endl;
	//now we are trying to get the first transform
	//just for the debug
	std::fstream fs;
	Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
	fs.open("/tmp/debug_tree_mat", std::ios::out);
	for (uint i = 0; i < this->bones.size(); i++) {
		this->cascade_transforms[i] = this->bones[i].getStackedTransformMat() * this->bones[i].offsetMat();
		fs << this->bones[i].id << std::endl;
		fs << glmMat2EigenMat(this->bones[i]._model_mat).format(CleanFmt) << "\n_________________________\n";
	}
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
	return true;
}

void
Skeleton::loadBoneWeights(const aiScene *s, int meshi, int bonej)
{
	cv::Mat_<float>& weights = this->mb_weights[meshi];
	cv::Mat_<int>& indices = this->mb_indices[meshi];
	aiMesh *mesh = s->mMeshes[meshi];
	aiBone *bone = mesh->mBones[bonej];
	//find the correct id of that bone
	std::string bone_name(bone->mName.data);
	int j  = this->bone_names[bone_name];
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
		const auto itr = this->bone_names.find(potential_bone);
		if (itr != this->bone_names.end()) {
			return current;
		}
		for (uint i = 0; i < current->mNumChildren; i++) {
			node_queue.push(current->mChildren[i]);
		}
	}
	return NULL;
}

//this is safe only because we insert all the bones before calling this function
void
Skeleton::buildHierachy(const aiScene *scene, const aiNode *root_node)
{
	std::queue<const aiNode *> fifo_nodes;
	this->root_bone = &this->bones[this->bone_names[root_node->mName.data]];
	fifo_nodes.push(root_node);
	while (!fifo_nodes.empty()) {
		const aiNode *node = fifo_nodes.front();
		fifo_nodes.pop();
		std::string name = node->mName.data;
		std::string parent_name = node->mParent->mName.data;
		Bone& thisbone = this->bones[bone_names[name]];
		thisbone._model_mat = aiMat2glmMat(node->mTransformation);
		//setup the parent first
		thisbone.parent = (this->bone_names.find(parent_name) != this->bone_names.end()) ?
			&this->bones[this->bone_names[parent_name]] : NULL;
		//now the children
		for (uint i = 0; i < node->mNumChildren; i++) {
			std::string childname = node->mChildren[i]->mName.data;
			if (this->bone_names.find(childname) != this->bone_names.end()) {
				thisbone.children.push_back(&this->bones[this->bone_names[childname]]);
				fifo_nodes.push(node->mChildren[i]);
			}
		}
	}
}


void
Skeleton::draw(const msg_t msg)
{
	const ShaderMan *sm = this->getBindedShader();
	for (uint i = 0; i < this->bones.size(); i++) {
		this->cascade_transforms[i] = this->bones[i].getStackedTransformMat() * this->bones[i].offsetMat();
	}
	sm->useProgram();

	glUniformMatrix4fv(glGetUniformLocation(sm->getPid(), this->uniform_bone.c_str()),
			   this->cascade_transforms.size(), //should be number of matrices
			   GL_FALSE,
			   (GLfloat *)&this->cascade_transforms[0]);
	//now we can draw all the meshes
	static_cast<Mesh1*>(this->model->searchProperty("mesh"))->draw(msg);
}
