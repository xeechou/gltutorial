#include <data.hpp>
#include <collections/geometry.hpp>


CubeModel::CubeModel(const glm::vec3 translation, const glm::vec3 scale, const glm::quat rotation)
{
	this->addProperty("mesh", std::make_shared<Mesh1>(CUBEVERTS, CUBENORMS, CUBETEXS, 36));
}
