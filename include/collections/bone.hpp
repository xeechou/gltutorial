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
	glm::mat4 _origin_mod; //initial pos
	glm::mat4 _invTransform;
	glm::mat4 _offset;
	RST trans;
public:
	Bone(const std::string id, const glm::mat4& offset, const glm::mat4& m = glm::mat4(1.0f));
	Bone(const Bone&& bone);
	~Bone(void);
	glm::mat4 updateInversTransform();
	glm::mat4 getInversTransform(void) const;
	glm::mat4 getoffset(void) const;
};

class Skeleton : public OBJproperty {
	//this one has normals, or mesh
protected:
	//in the end, we end up with the same structure of previous one
	std::string uniform_bone;
	//just for the sake of convience
	std::map<std::string, int> bone_names;
	std::vector<Bone> bones;
	//this is the GPU data
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
	virtual void draw(const msg_t) override;
	//we need a way to debug it.
};

#endif
