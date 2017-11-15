#include <types.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <random>

#include <types.hpp>
#include <property.hpp>
#include <shaderman.h>
#include <operations.hpp>

Transforming::Transforming(const glm::vec3& t, const glm::vec3& a, const glm::vec3& s) :
	translation(t), rotation(glm::quat(a)), scaling(s)
{

	this->modelMat = glm::translate(this->translation) *
		this->rotaiton.toMat4() * glm::scale(this->scaling);
}

//
void
Transforming::transform(const glm::vec3& t,
			const glm::vec3& ang
			const glm::vec3& s)
{
	this->translation += t;
	this->scaling += s;
	//for rotation. I need to figure out the how to add two quaternion
}



void
Transforming::draw(const msg_t msg)
{
	(void)msg;
	const ShaderMan *prog = this->getBindedShader();
	prog->useProgram();
	glUniform4m();
}
