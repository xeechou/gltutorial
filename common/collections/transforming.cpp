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


void
Transforming::draw(const msg_t msg)
{
	(void)msg;
	const ShaderMan *prog = this->getBindedShader();
	prog->useProgram();
	glUniform4m();
}
