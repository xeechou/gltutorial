#ifndef OPERATIONS_HPP
#define OPERATIONS_HPP

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

#endif
