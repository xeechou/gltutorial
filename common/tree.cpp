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

#include <tree.hpp>

TreeNode::TreeNode(const std::string id, const glm::mat4& m) :
	_model_mat(m)
{
	if (id == "")
		this->id = "accun Nom";
	else
		this->id = id;
	this->parent = NULL;
	this->children.clear();
}

//we should define a static function here
//static const std::string&
std::string TreeNode::layout() const
{
	typedef std::pair<int, const TreeNode *> indent_node_t;
	
	std::stringstream ss;
	std::stack<indent_node_t> nodes;
	nodes.push(std::make_pair(0, this));
	while(!nodes.empty()) {
		int indent = nodes.top().first;
		const TreeNode *node = nodes.top().second;
		ss << std::string(" ", indent);
		ss << node->name() << std::endl;
		nodes.pop();
		for (unsigned int i = 0; i < node->children.size(); i++)
			nodes.push(std::make_pair(indent + 2, node->children[i]));
	}
	std::string s;
	return ss.str();
}
