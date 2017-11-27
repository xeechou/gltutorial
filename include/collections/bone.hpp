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
	//the finally should be all stacked transform * _offsetMat
	///matrix that transfer the vertices from model space to bone space.
	glm::mat4 _offsetMat;
	//the index of the bone
	int _index;
	glm::mat4 _invTransform;
public:
	Bone(int indx, const std::string id = "", const glm::mat4& m = glm::mat4(1.0f));
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
	std::string uniform_bone;
	std::map<std::string, Bone> bones;
	std::vector<glm::mat4> cascade_transforms;
	const Bone *root_bone;
	//in the mean time, we should also keep the a sparseMatrix of the bone weights
	std::vector<cv::Mat_<float> > mb_weights;
	std::vector<cv::Mat_<int> > mb_indices;
	std::vector<GLuint> gpu_handles;

	void loadBoneWeights(const aiScene *scene, int mesh, int bone);
	aiNode* findRootBone(const aiScene *scene) const;
	void buildHierachy(const aiScene *scene, const aiNode *root);
public:
	Skeleton(uint weights=3, const std::string& uniform_name=UNIFORM_BONE_ARR);
	virtual ~Skeleton() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
//	virtual bool uploadUniform(const ShaderMan* sm) override;
	virtual void draw(const msg_t) override;
};

#endif
