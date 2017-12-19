#include <data.hpp>
#include <types.hpp>
#include <operations.hpp>
#include <collections/geometry.hpp>


CubeModel::CubeModel(const glm::vec3 translation, const glm::vec3 scale, const glm::quat rotation)
{
	this->addProperty("mesh", std::make_shared<Mesh1>(CUBEVERTS, CUBENORMS, CUBETEXS, 36));
}


isoSphere::isoSphere(float radius)
{
	//also we need the degree to be divisible by 180
	int smallest_degree = find_lager_divisor(180, std::max(1, (int)(20 / radius)));

	size_t nlats = (int)(180 / smallest_degree);
	size_t nlongs = (int)(360 / smallest_degree) - 1;
	long latitudes[nlats];
	int n = 0;
	std::generate(latitudes, latitudes+nlats, [&]() {
			return -90 + smallest_degree * n++;
		});
	n = 0;
	latitudes[nlats-1] = glm::quarter_pi<float>()*2;
	long longtitudes[nlongs];
	std::generate(longtitudes, longtitudes+nlongs, [&]() {return smallest_degree * n++;});

	std::vector<glm::vec3> vertices((nlats-2) * nlongs + 2);
	vertices[0] = glm::vec3(0.0f, -radius, 0.0f);
	for (uint i = 0; i < nlongs; i++) {
		for(uint j = 1; j < nlats-1; j++) {
			int idx = i * (nlats-2) + j;
			vertices[idx] = polar2euclidean(radius, longtitudes[i], latitudes[j]);
		}
	}
	vertices[(nlats-2)*nlongs+1] = glm::vec3(0.0f, radius, 0.0f);
	//now generate the element buffer
	std::vector<face_t> element_buffer;
	for (uint i = 0; i < nlongs; i++) {
		uint j = 1;
		int idx = i * (nlats-2)+j;
		int nextidx = ((i+1) % nlongs) * (nlats-2)+j;
		element_buffer.push_back(face_t(idx, 0, nextidx));

		for (; j < nlats-2; j++) {
			idx = i * (nlats-2) + j;
			nextidx = ((i+1) % nlongs) *(nlats-2)+j;
			element_buffer.push_back(face_t(idx, nextidx, nextidx+1));
			element_buffer.push_back(face_t(nextidx+1, idx+1, idx));
		}
		idx = i * (nlats-2)+j;
		nextidx = ((i+1) % nlongs) * (nlats-2)+j;
		element_buffer.push_back(glm::u32vec3(idx, vertices.size()-1, nextidx));
	} //I need to test it
	this->addProperty("mesh", std::make_shared<Mesh1>(std::move(vertices), std::move(element_buffer)));
}
