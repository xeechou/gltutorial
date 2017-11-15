#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

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
trs2mat4(const glm::vec& t, const glm::quat& r, const glm::vec3& s)
{
	return glm::translate(t) * r.toMat4() * glm::scale(s);
}


#endif
