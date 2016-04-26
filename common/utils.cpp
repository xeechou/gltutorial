#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <utils.h>

#include <GL/glew.h>
#include <assert.h>
//return the size of the buffer
static int
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

GLuint load_shader(const char *fname, GLenum type)
{
	
	std::string shader_code;
	if (read_file(fname, &shader_code)) {
		std::cout << "do you even have the shader file?" << std::endl;
		return 0;
	}

	int loglen = 0;
	GLint result = GL_FALSE;
	GLuint sid = glCreateShader(type);
	const char *shader_code_pointer = shader_code.c_str();
	glShaderSource(sid, 1, &shader_code_pointer, NULL);
	glCompileShader(sid);

	//check compile status
	glGetShaderiv(sid, GL_COMPILE_STATUS, &result);
	glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &loglen);
	if (result != GL_TRUE) {
//		std::cout<<"there shoud be something" << glGetError()<<std::endl;
		std::vector<char> err_msg(loglen+1);
		glGetShaderInfoLog(sid, loglen, NULL, &err_msg[0]);
		fprintf(stderr, "shader Compile info: %s\n", &err_msg[0]);
		return 0;
	}
	return sid;
}
//the interface may change
GLuint load_shader_program(GLuint *shaders, int len)
{
	GLint result;
	int loglen;
	GLuint prog_id = glCreateProgram();
	for (int i = 0; i < len; i++) {
		glAttachShader(prog_id, shaders[i]);		
	}

	glLinkProgram(prog_id);
	glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
	glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &loglen);
	if (result != GL_TRUE){
		std::vector<char> ProgramErrorMessage(loglen+1);
		glGetProgramInfoLog(prog_id, loglen, NULL, &ProgramErrorMessage[0]);
		std::cout << "Program Compile info: " << &ProgramErrorMessage[0] << std::endl;
		return 0;
	}
	//check program
	return prog_id;
}
