#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>


class TreeNode {
public:
	std::string id;
	TreeNode *parent;
	std::vector<TreeNode*> children;
	glm::mat4 _model_mat;
	TreeNode(const std::string id = "", const glm::mat4& m = glm::mat4(1.0f));
	const std::string name() const {return id;}
	glm::mat4 getModelMat() const {return _model_mat;}
	void setModelMat(glm::mat4& model) {_model_mat = model;}
	glm::mat4 getStackedTransformMat() const;
	/**
	 * @brief this node and all its decedents
	 */
	std::string layout() const;
};
