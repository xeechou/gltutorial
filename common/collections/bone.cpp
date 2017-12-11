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


Bone::Bone(const std::string id, const glm::mat4& offset, const glm::mat4& model_mat) :
	TreeNode(id), _offset(offset)
{
	_model_mat = model_mat;
}

Bone::Bone(const Bone&& bone)
{
	this->id = bone.id;
	this->_offset = bone._offset;
	this->_model_mat = bone._model_mat;
}

Bone::~Bone()
{
}

glm::mat4
Bone::getoffset() const
{
	return this->_offset;
}

glm::mat4
Bone::updateInversTransform()
{

	this->_invTransform =  glm::inverse(this->updateStackedTransformMat());
	return this->_invTransform;
}

glm::mat4
Bone::getInversTransform() const
{
	return this->_invTransform;
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
	this->bone_weights.resize(scene->mNumMeshes);
	for (uint i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[i];
		//initialize this first
		this->bone_weights[i] = Skeleton::bw_mat_t::zeros(mesh->mNumVertices, this->shader_layouts.second);
		std::vector<uint> weight_indx(mesh->mNumVertices, 0);
		for (uint j = 0; j < mesh->mNumBones; j++) {
			const std::string bone_name = std::string(mesh->mBones[j]->mName.C_Str());
			aiBone *aibone = mesh->mBones[j];
			if (this->bone_names.find(bone_name) == this->bone_names.end()) {
				size_t bone_idx = this->bones.size();
				this->bone_names[bone_name] = bone_idx;
				this->bones.emplace_back( std::move(Bone(bone_name, aiMat2glmMat(aibone->mOffsetMatrix))) );
				//load bone_weights
				for (uint k = 0; k < aibone->mNumWeights; k++) {
					uint vid = aibone->mWeights[k].mVertexId;
					float w  = aibone->mWeights[k].mWeight;
					this->bone_weights[i](vid, weight_indx[vid]++) = cv::Vec2f((float)bone_idx, w);
				}
			}
		}
	}
	this->buildHierachy(scene, findRootBone(scene));
	this->cascade_transforms.resize(this->bones.size());
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
static void
normalize_weights(cv::Mat_<cv::Vec2f>& mat)
{
	cv::Mat channels[2];
	cv::split(mat, channels);
	cv::Mat tmprow(1,channels[1].cols, channels[1].type());
	for (int i = 0; i < channels[1].rows; i++)
		cv::normalize(channels[1].row(i), channels[1].row(i), 1,0,cv::NORM_L1);
	cv::merge(channels, 2, mat);
}


bool
Skeleton::push2GPU()
{
	this->gpu_handles.resize(this->bone_weights.size());
	uint first_layout = this->shader_layouts.first;
	uint layout_ends  = this->shader_layouts.first + this->shader_layouts.second;
	Mesh1* mesh_handle = (Mesh1*)this->model->searchProperty("mesh");
	size_t vec2_size = sizeof(cv::Vec2f);
	for (uint i = 0; i < this->bone_weights.size(); i++) {
		//NOTE after assembled weight, we still have float error problems
		normalize_weights(this->bone_weights[i]);
//		debug_bw(bw);
		mesh_handle->activeIthMesh(i);
		glGenBuffers(1, &this->gpu_handles[i]);
		glBindBuffer(GL_ARRAY_BUFFER, this->gpu_handles[i]);
		glBufferData(GL_ARRAY_BUFFER, this->bone_weights[i].step[0] * this->bone_weights[i].rows,
			     (void *)this->bone_weights[i].data, GL_STATIC_DRAW);

		for (uint l = first_layout; l < layout_ends; l++) {
			glEnableVertexAttribArray(l);
			glVertexAttribPointer(l, 2, GL_FLOAT, GL_FALSE,
					      this->bone_weights[i].step[0],
					      (void *)(l * vec2_size));
		}
		glBindVertexArray(0);
	}
	return true;
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
		thisbone.updateInversTransform();

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
//	((Bone*)this->root_bone)->setModelMat(glm::eulerAngleXYZ(34.0f, 20.0f,0.0f));
	for (uint i = 0; i < this->bones.size(); i++) {
		//seems most likely there are
		this->cascade_transforms[i] = this->bones[i].getInversTransform() *this->bones[i].updateStackedTransformMat();// * this->bones[i].getoffset();
		this->cascade_transforms[i] = this->bones[i].updateStackedTransformMat() * this->bones[i].getInversTransform();
			//this->bones[i].getInversTransform() * //this->bones[i].getoffset();
	}
	sm->useProgram();
	glUniformMatrix4fv(glGetUniformLocation(sm->getPid(), this->uniform_bone.c_str()),
			   this->cascade_transforms.size(), //should be number of matrices
			   GL_FALSE,
			   (GLfloat *)&this->cascade_transforms[0]);
	//now we can draw all the meshes
	static_cast<Mesh1*>(this->model->searchProperty("mesh"))->draw(msg);
}
