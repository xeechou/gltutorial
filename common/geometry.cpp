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
	int smallest_degree = find_lager_divisor(180, std::max(1, (int)(10 / radius)));
	//nlats is actually n+1, we have the begining and the end, that is why we need +1
	size_t nlats = (int)(180 / smallest_degree)+1;
	size_t nlongs = (int)(360 / smallest_degree);
	long latitudes[nlats];
	int n = 0;
	//datas
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texuvs;

	std::generate(latitudes, latitudes+nlats,
		      [&]() { return -90 + smallest_degree * n++;});
	n = 0;
	latitudes[nlats-1] = glm::quarter_pi<float>()*2;
	long longtitudes[nlongs];
	std::generate(longtitudes, longtitudes+nlongs, [&]() {return smallest_degree * n++;});

	vertices.resize((nlats-2) * nlongs + 2);
	normals.resize((nlats-2) * nlongs + 2);
	texuvs.resize((nlats-2) * nlongs + 2);
	vertices[0] = glm::vec3(0.0f, -radius, 0.0f);
	normals[0] = glm::normalize(vertices[0]);
	texuvs[0] = glm::vec2(0.5, 0);
	for (uint i = 0; i < nlongs; i++) { //[0, theta, ..., 180 - theta]
		for(uint j = 1; j < nlats-1; j++) { //[theta, ... , 360 - theta]
			int idx = i * (nlats-2) + j;
			vertices[idx] = polar2euclidean(radius, longtitudes[i], latitudes[j]);
			normals[idx] = glm::normalize(vertices[idx]);
			texuvs[idx] = glm::vec2((float)i/nlongs, (float)j / nlats );
		}
	}
	vertices[(nlats-2)*nlongs+1] = glm::vec3(0.0f, radius, 0.0f);
	normals[(nlats-2)*nlongs +1] = glm::normalize(vertices[(nlats-2)*nlongs+1]);
	texuvs[(nlats-2)*nlongs+1] = glm::vec2(0.5, 1.0);

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
	this->addProperty("mesh", std::make_shared<Mesh1>(std::move(vertices),
							  std::move(element_buffer),
							  std::move(normals),
							  std::move(texuvs)));
}
