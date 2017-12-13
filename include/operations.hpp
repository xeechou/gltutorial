#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

//requires c++14 code
constexpr bool
cstr_eq(const char *lhs, const char *rhs)
{
	while (*lhs || *rhs)
		if (*lhs++ != *rhs++)
			return false;
	return true;
}

//write as much c++11 code as possible

static inline glm::mat4
trs2mat4(const glm::vec3& t, const glm::quat& r, const glm::vec3& s)
{
	return glm::translate(t) * glm::mat4_cast(r) * glm::scale(s);
}


static inline void
print_glmMat4(const glm::mat4& mat)
{
	for (int i = 0; i < 4; i++)
		std::cout << glm::to_string(glm::row(mat,i)) << std::endl;
	std::cout << std::endl;
}

static inline void
print_glmMat3(const glm::mat3& mat)
{
	for (int i = 0; i < 3; i++)
		std::cout << glm::to_string(glm::row(mat,i)) << std::endl;
	std::cout << std::endl;
}

/**
 * (latitude, longtitude, radius) to world coordinates
 */
static inline glm::vec3
polar2euclidean(const glm::vec3& p)
{
	return p[2] * glm::vec3(glm::cos(p[0]) * glm::sin(p[1]),
				glm::cos(glm::quater_pi<float>() - p[0]),
				glm::sin(p[1]) * glm::cos(p[0]));
}


#endif
