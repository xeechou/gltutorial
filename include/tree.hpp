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
	glm::mat4 _cascade_transform;
	
	TreeNode(const std::string id = "", const glm::mat4& m = glm::mat4(1.0f));
	~TreeNode(void);
	
	const std::string name() const {return id;}
	const glm::mat4 getModelMat() const;
	void setModelMat(const glm::mat4& model);
	/**
	 * @brief accumlate the transformation matrix from its accenster, setModelMat has to call first
	 */
	const glm::mat4 getStackedTransformMat() const;
	/**
	 * @brief set the cascade transformation and its inverse for the this node. It will prove useful
	 */
	virtual void setStackedTransformMat();
	/**
	 * @brief flush the transformation from this nodes and all its descedens
	 */
	void flushTransformations();
	std::string layout() const;
};
