#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <random>

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



JointTransform::JointTransform(const glm::vec3& t, const glm::quat& r, const glm::vec3& s)
{
	this->translation  = t;
	this->rotation  = r;
	this->scale = s;
}

JointTransform
JointTransform::interpolate(const JointTransform &a, const JointTransform &b, float progression)
{
	glm::vec3 trans = (1-progression) * a.translation + progression * b.translation;
	glm::vec3 scale = (a.scale != b.scale) ?
		((1-progression) * a.scale + progression * b.scale)
		: a.scale;
	glm::quat quaternion = glm::slerp(a.rotation, b.rotation, progression);
	return JointTransform(trans, quaternion, scale);
}


Bone::Bone(const std::string id, const glm::mat4& m) : TreeNode(id, m)
{
}

Bone::Bone(const Bone& bone) : TreeNode(bone) {
	_offsetMat = bone._offsetMat;
}

void
Bone::setStackedTransformMat()
{
	TreeNode::setStackedTransformMat();
	this->_invTransform = glm::inverse(this->_cascade_transform);
}


