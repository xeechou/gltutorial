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
	this->shader_layouts.second = 1;
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

//this is where you found out the importance of order
bool
Instancing::push2GPU(void)
{
	if (this->instanceVBO)
		glDeleteBuffers(1, &this->instanceVBO);
	
	Mesh1* mesh_handle = (Mesh1*)this->model->searchProperty("mesh");
	
	std::vector<glm::vec3>& trs = this->rsts.translations;
	std::vector<glm::quat>& rts = this->rsts.rotations;
	std::vector<glm::vec3>& scs = this->rsts.scales;

	size_t ninstances = trs.size();
	std::vector<glm::mat4> instance_mats(ninstances);
	for (size_t i = 0; i < instance_mats.size(); i++)
		instance_mats[i] = glm::translate(trs[i]) * glm::mat4_cast(rts[i]) * glm::scale(scs[i]);
	
	glGenBuffers(1, &this->instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * instance_mats.size(), &instance_mats[0], GL_STATIC_DRAW);
	
	

	
	return true;
}
