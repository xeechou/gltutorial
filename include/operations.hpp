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

static inline bool
is_divisor(long multiple, int div)
{
	return (multiple % div == 0) ? true : false;
}

static inline int
find_lager_divisor(long multiple, int div)
{
	return (multiple % div == 0) ? div : find_lager_divisor(multiple, div+1);
}

static inline int
find_smaller_divisor(long multiple, int div)
{
	return (multiple % div == 0) ? div : find_smaller_divisor(multiple, div-1);
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
 * (raidus, longtitude, latitude) to world coordinates
 */
static inline glm::vec3
polar2euclidean(float r, float theta, float phi)
{
	return r * glm::vec3(glm::cos(theta) * glm::cos(phi),
			     glm::sin(phi),
			     glm::sin(theta) * glm::cos(phi));
}

/**
 * degree version
 */
static inline glm::vec3
polar2euclidean(float r, long theta, long phi)
{
	float rt = glm::radians((float)theta);
	float rp = glm::radians((float)phi);
	return r * glm::vec3(glm::cos(rt) * glm::cos(rp),
			     glm::sin(rp),
			     glm::sin(rt) * glm::cos(rp));
}

/*
static inline glm::vec3
euclidean2poloar(float x, float y, float z)
{
	float r = glm::l2Norm(glm::vec3(x,y,z));
}
*/
#endif
