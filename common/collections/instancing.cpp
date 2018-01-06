#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <random>

#include <types.hpp>
#include <property.hpp>
#include <model.hpp>
#include <shaderman.h>


Instancing::Instancing(const RSTs& copy)
{
	this->rsts = copy;
	this->shader_layouts.second = 4; //we need to allocate for mat4
}

Instancing::Instancing(const int n, const OPTION opt,
		       const glm::vec3 dscale, const glm::vec3 dtrans, const glm::quat droate)
{
	(void)dtrans;
	if (opt == Instancing::OPTION::square_instances) {
		this->rsts.translations.resize(n * n);
		this->rsts.scales.resize(n * n);
		this->rsts.rotations.resize(n * n);

		int rows  = n;
		int cols  = n;
		int count = 0;
		for (int i = 0; i < rows; i++) {
			for( int j = 0;  j < cols; j++) {
				this->rsts.translations[count] = glm::vec3((float)i, 0.0f, (float)j);
				this->rsts.rotations[count] = droate;
				this->rsts.scales[count] = dscale;
				count += 1;
			}
		}

	}
	else if (opt == Instancing::OPTION::random_instances) {
		this->rsts.translations.resize(n);
		this->rsts.scales.resize(n);
		this->rsts.rotations.resize(n);
		std::random_device rd;
		std::minstd_rand el(rd());

		for (int i = 0; i < n; i++) {
			this->rsts.translations[i] = glm::vec3(el(), el(), el());
			this->rsts.rotations[i] = droate;
			this->rsts.scales[i] = dscale;
		}
	}

}

Instancing::~Instancing()
{
	if (this->instanceVBO)
		glDeleteBuffers(1, &this->instanceVBO);
}

//this function has to call after mesh
bool
Instancing::push2GPU(void)
{
	if (this->instanceVBO)
		glDeleteBuffers(1, &this->instanceVBO);

	Mesh1* mesh_handle = (Mesh1*)this->model->searchProperty("mesh");
	size_t vec4_size = sizeof(glm::vec4);

	std::vector<glm::vec3>& trs = this->rsts.translations;
	std::vector<glm::quat>& rts = this->rsts.rotations;
	std::vector<glm::vec3>& scs = this->rsts.scales;

	size_t ninstances = trs.size();
	std::vector<glm::mat4> instance_mats(ninstances);
	for (size_t i = 0; i < instance_mats.size(); i++) {
		instance_mats[i] = glm::translate(trs[i]) * glm::mat4_cast(rts[i]) * glm::scale(scs[i]);
//		std::cout << glm::to_string(instance_mats[i]) << std::endl;
	}

	glGenBuffers(1, &this->instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instance_mats.size(), &instance_mats[0], GL_STATIC_DRAW);
	//this is the
	for (uint i = 0; i < mesh_handle->howmanyMeshes(); i++) {
		mesh_handle->activeIthMesh(i);
		uint ilayout = this->shader_layouts.first;
		glEnableVertexAttribArray(ilayout);
		glVertexAttribPointer(ilayout, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)0);
		glEnableVertexAttribArray(ilayout+1);
		glVertexAttribPointer(ilayout+1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)vec4_size);
		glEnableVertexAttribArray(ilayout+2);
		glVertexAttribPointer(ilayout+2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(2*vec4_size));
		glEnableVertexAttribArray(ilayout+3);
		glVertexAttribPointer(ilayout+3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(3*vec4_size));
		glVertexAttribDivisor(ilayout,   1);
		glVertexAttribDivisor(ilayout+1, 1);
		glVertexAttribDivisor(ilayout+2, 1);
		glVertexAttribDivisor(ilayout+3, 1);

		glBindVertexArray(0);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}


void
Instancing::draw(const msg_t msg)
{
	uint indices_size = msg.u;
	glDrawElementsInstanced(GL_TRIANGLES, indices_size, GL_UNSIGNED_INT, 0, this->rsts.translations.size());
}
