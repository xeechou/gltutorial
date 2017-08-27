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

Bone::Bone(const std::string id, const glm::mat4& m) : TreeNode(id, m)
{
	
}
Bone::Bone(const Bone& bone) : TreeNode(bone) {
	_offsetMat = bone._offsetMat;
}

