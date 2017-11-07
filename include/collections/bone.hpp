#ifndef BONE_HPP
#define BONE_HPP


#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include "../tree.hpp"
#include "../types.hpp"

//##include <model.hpp>
#include "../shaderman.h"
#include "../data.hpp"
#include "../property.hpp"

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
	Bone(const Bone&& bone);
	Bone(void);
	~Bone(void);
	int getInd() const {return _index;}
	void setInd(int ind) {_index = ind;}
	void setStackedTransformMat() override;
};

class Skeleton : public OBJproperty {
	//this one has normals, or mesh
protected:
	//in the end, we end up with the same structure of previous one
	std::map<std::string, Bone> bones;
	const Bone *root_bone;
	//in the mean time, we should also keep the a sparseMatrix of the bone weights
	std::vector<cv::Mat_<float> > mb_weights;
	std::vector<cv::Mat_<int> > mb_indices;
	std::vector<GLuint> gpu_handles;

	void loadBoneWeights(const aiScene *scene, int mesh, int bone);
	aiNode* findRootBone(const aiScene *scene) const;
	void buildHierachy(const aiScene *scene, const aiNode *root);
public:
	Skeleton(uint weights=3);
	virtual ~Skeleton() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
};

#endif
