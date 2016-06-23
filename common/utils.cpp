#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <utils.h>

#include <GL/glew.h>
#include <assert.h>

int
read_file(const char *fname, std::string *str)
{
	assert(str);
	std::stringstream ss;
	std::ifstream f(fname, std::ios::in);
	if (!f.is_open())
		return -1;

	ss << f.rdbuf();
	str->assign(ss.str());
	f.close();
	return 0;
}

//return the size of the buffer
