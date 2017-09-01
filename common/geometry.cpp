#include <data.hpp>
#include <collections/geometry.hpp>


CubeModel::CubeModel(const glm::vec3 translation, const glm::vec3 scale, const glm::quat rotation)
{
	this->instanceVBO = 0;
	this->meshes.push_back(Mesh(CUBEVERTS, CUBENORMS, CUBETEXS, 36));
	glm::quat default_rotation = glm::quat(glm::vec3(0.0f));
	
	glm::mat3 position_mat = glm::mat3_cast(rotation) * glm::mat3(glm::scale(scale));
	glm::mat3 normal_mat   = glm::transpose(glm::inverse(position_mat));
	
	std::vector<glm::vec3>& positions = this->meshes[0].vertices.Positions;
	std::vector<glm::vec3>& normals   = this->meshes[0].vertices.Normals;
	//see if we need to do anything
	if (translation == glm::vec3(0.0f) && scale == glm::vec3(1.0f) && rotation == default_rotation)
		return;
	
	for (size_t i = 0; i < positions.size(); i++) {
		positions[i] = translation + position_mat * positions[i];
		if (!normals.empty())
			normals[i] = normal_mat * normals[i];
	}

}
