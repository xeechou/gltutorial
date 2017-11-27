#include <iostream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <stack>
#include <utility>
#include <sstream>
#include <cstdio>

#include <types.hpp>
#include <tree.hpp>

TreeNode::TreeNode(const std::string id, const glm::mat4& m) :
	_model_mat(m)
{
	this->id = id;
	this->parent = NULL;
	this->children.clear();
	this->flushed = false;
}

TreeNode::TreeNode(void)
{
	std::cout << "called superclass constructor" << std::endl;
	this->id = "";
	this->parent = NULL;
	this->children.clear();
	this->_model_mat = glm::mat4(1.0f);
	this->flushed = false;
}

TreeNode::~TreeNode()
{
//	std::cerr << "called tree descturctor" << std::endl;
	//delete all its children. Since we don't use share_ptr here
//	for (uint i = 0; i< this->children.size(); i++)
//		delete this->children[i];
}

//we should define a static function here
std::string TreeNode::layout() const
{
	typedef std::pair<int, const TreeNode *> indent_node_t;

	std::stringstream ss;
	std::stack<indent_node_t> nodes;
	nodes.push(std::make_pair(0, this));
	while(!nodes.empty()) {
		int indent = nodes.top().first;
		const TreeNode *node = nodes.top().second;
		ss << std::string(indent, ' ');
		ss << node->name() << std::endl;
		nodes.pop();

		for (unsigned int i = 0; i < node->children.size(); i++)
			nodes.push(std::make_pair(indent + 2, node->children[i]));
	}
	std::string s;
	return ss.str();
}

const glm::mat4
TreeNode::getModelMat() const
{
	return _model_mat;
}

void
TreeNode::setModelMat(const glm::mat4& model)
{
	_model_mat = model;
	this->flushed = false;
}

const glm::mat4
TreeNode::getStackedTransformMat()
{
	if (!parent) {
		this->flushed = true;
		return this->_model_mat;
	} else {
		this->flushed = true;
		return this->parent->getStackedTransformMat() * _cascade_transform;
	}

}

void
TreeNode::setStackedTransformMat()
{
	if (!this->parent)
		this->_cascade_transform = this->_model_mat;
	else {
		this->_cascade_transform = this->_model_mat * this->parent->getStackedTransformMat();
	}
}

void
TreeNode::flushTransformations()
{
	//there should be something
	glm::mat4 parent_ctrans=glm::mat4(1.0);
	if (parent)
		parent_ctrans = parent->getStackedTransformMat();
	this->_cascade_transform = parent_ctrans * this->_model_mat;
	for (uint i = 0; i < this->children.size(); i++)
		children[i]->flushTransformations();
}
