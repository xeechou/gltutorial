#include <data.hpp>
#include <collections/geometry.hpp>


CubeModel::CubeModel(const glm::vec3 translation, const glm::vec3 scale, const glm::quat rotation)
{
	this->addProperty("mesh", std::make_shared<Mesh1>(CUBEVERTS, CUBENORMS, CUBETEXS, 36));
}


isoSphere::isoSphere(float radius)
{
	float smallest_degree = 0.1 / radius;
	size_t nlats = (int)(glm::pi<float>() / smallest_degree);
	size_t nlongs = (int)(glm::pi<float>() * 2 / smallest_degree) - 1;
	size_t latitudes[nlats];
	int n = 0;
	std::generate(latitudes, latitudes+nlats, [&]() {
			return -glm::quarter_pi<float>() * 2.0 + smallest_degree * n++;
		});
	latitudes[nlats-1] = glm::quarter_pi<float>()*2;
	size_t longtitudes[nlongs];
	n = 0;
	std::generate(longtitudes, longtitudes+nlongs, [&]() {return smallest_degree * n++;});
	std::vector<glm::vec3> vertices((nlats-2) * nlongs + 2);
	vertices[0] = glm::vec3(0.0f, -radius, 0.0f);
	for (int i = 0; i < nlongs; i++) {
		for( int j = 1; j < nlats-1; j++) {
			int idx = i * (nlats-2) + j;
			vertices[idx] = polar2euclidean(radius, longtitudes[i], latitudes[j]);
		}
	}
	vertices[(nlats-2)*nlongs+1] = glm::vec3(0.0f, radius, 0.0f);
	//now generate the element buffer
	std::vector<glm::u32vec3> element_buffer;
	for (int i = 0; i < nlongs; i++) {
		int j = 1;
		int idx = i * (nlats-2)+j;
		int nextidx = ((i+1) % nlongs) * (nlats-2)+j;
		element_buffer.push_back(glm::u32vec3(idx, 0, nextidx));

		for (; j < nlats-2; j++) {
			idx = i * (nlats-2) + j;
			nextidx = ((i+1) % nlongs) *(nlats-2)+j;
			element_buffer.push_back(glm::u32vec3(idx, nextidx, nextidx+1));
			element_buffer.push_back(glm::u32vec3(nextidx+1, idx+1, idx));
		}
		idx = i * (nlats-2)+j;
		nextidx = ((i+1) % nlongs) * (nlats-2)+j;
		element_buffer.push_back(glm::u32vec3(idx, vertices[vertices.size()-1], nextidx));
	} //I need to test it
//	this->addProperty("mesh", std::shared_ptr<Mesh1(const float *vertx, const float *norms, const float *uvs, const int nnodes)>);
}
