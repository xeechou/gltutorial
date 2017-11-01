#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <random>

#include <tree.hpp>
#include <types.hpp>
#include <property.hpp>
#include <model.hpp>
#include <shaderman.h>
#include <data.hpp>

//I think this is fair
class Bone : public TreeNode {
	///matrix that transfer the vertices from mesh(world) space to bone space.
	glm::mat4 _offsetMat;
	//the index of the bone
	int _index;
	glm::mat4 _invTransform;
public:
	Bone(int indx, const std::string id = "", const glm::mat4& m = glm::mat4(1.0f));
	Bone(const Bone& bone);
	~Bone(void);
	int getInd() const {return _index;}
	void setInd(int ind) {_index = ind;}
	void setStackedTransformMat() override;
};

Bone::Bone(int indx, const std::string id, const glm::mat4& m) :
	TreeNode(id), _offsetMat(m), _index(indx)
{
	//we are setting up the stacked transform later
}

Bone::Bone(const Bone& bone) : TreeNode(bone)
{
	//we cannot assume it has other code
	_offsetMat = bone._offsetMat;
	_index = bone._index;
	id = bone.id;
}

void
Bone::setStackedTransformMat()
{
	TreeNode::setStackedTransformMat();
	this->_invTransform = glm::inverse(this->_cascade_transform);
}


typedef std::vector<Eigen::Triplet<float> > BoneWeights;

class Skeleton : public OBJproperty {
	//this one has normals, or mesh
protected:
	std::map<std::string, Bone> bones;
	//in the mean time, we should also keep the a sparseMatrix of the bone weights
	std::vector<BoneWeights> meshes;
	void loadBoneWeights(const aiScene *scene, int mesh, int bone);
	aiNode* findRootBone(const aiScene *scene) const;
	void buildHierachy(const aiScene *scene, const aiNode *root);
public:
	Skeleton(uint weights=3);
	virtual ~Skeleton() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
};

Skeleton::Skeleton(uint weights)
{
	this->shader_layouts.second=weights;
}

bool
Skeleton::load(const aiScene *scene)
{
	this->meshes.resize(scene->mNumMeshes);

	for (uint i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[i];
		for (uint j = 0; j < mesh->mNumBones; j++) {
			const std::string bone_name = std::string(mesh->mBones[j]->mName.C_Str());
			aiBone *aibone = mesh->mBones[j];
			if (this->bones.find(bone_name) == this->bones.end()) {
				Bone localbone(this->bones.size(), bone_name, aiMat2glmMat(aibone->mOffsetMatrix));
				this->bones.insert(std::make_pair(bone_name, localbone));
			}
							  
			this->loadBoneWeights(scene, i, j);
		}
	}
	//all right, since I loaded up all the meshes
	aiNode *rootNode = this->findRootBone(scene);
	assert(rootNode != NULL);
	
	return true;
}

bool
Skeleton::push2GPU()
{
	//what to push to GPU?
	//alright, we shoudl 
}

void
Skeleton::loadBoneWeights(const aiScene *s, int meshi, int bonej)
{
	BoneWeights& weights = this->meshes[meshi];
	aiMesh *mesh = s->mMeshes[meshi];
	aiBone *bone = s->mMeshes[meshi]->mBones[bonej];
	std::string bone_name = std::string(bone->mName.C_Str());
	//find the correct id of that bone
	int j;
	for (j = 0; j < this->bones.size(); j++)
		if (bones[j].name() == bone_name)
			break;
	
	typedef Eigen::Triplet<float> T;
	for (int i = 0; i < bone->mNumWeights; i++)
		weights.push_back(T(bone->mWeights[i].mVertexId, j,
				    bone->mWeights[i].mWeight));
	//will create the SparseMat on push2GPU()
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
