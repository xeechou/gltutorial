#include <types.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>
#include <random>

#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif


#include <types.hpp>
#include <property.hpp>
#include <shaderman.h>
#include <operations.hpp>

Transforming::Transforming(const glm::vec3& t, const glm::vec3& a, const glm::vec3& s) :
	translation(t), rotation(a), scaling(s)
{
}
Transforming::Transforming(float pitch, float yall, float roll)
{
	this->translation = glm::vec3(0.0);
	this->rotation = glm::vec3(pitch, yall, roll);
	this->scaling = glm::vec3(1.0);
}




glm::mat4
Transforming::getMMat()
{
	this->modelMat = glm::translate(this->translation) *
		glm::eulerAngleXYZ(this->rotation[0], this->rotation[1], this->rotation[2]) *
		glm::scale(this->scaling);
	return this->modelMat;
}

void
Transforming::transform(const glm::vec3& t,
			const glm::vec3& ang,
			const glm::vec3& s)
{
	this->translation += t;
	this->scaling += s;
	this->rotation += ang;
}

void
Transforming::rotate(float pitch, float yall, float roll)
{
	this->rotation += glm::vec3(pitch, yall, roll);
}
/*
void
Transforming::draw(const msg_t msg)
{
	(void)msg;
	const ShaderMan *prog = this->getBindedShader();
	prog->useProgram();
	glUniformMatrix4fv( glGetUniformLocation(prog->getPid(),
						 this->uniform.c_str()),
			    1, GL_FALSE, &this->modelMat[0][0]);
}
*/
