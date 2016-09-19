#include <sstream>
#include <iostream>
#include <GL/glew.h>
#include <stdio.h>
//#include <shaderman.h>
#include "../include/shaderman.h"
#include "../include/utils.h"

static
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
		fprintf(stderr, "%s shader Compile info: %s\n",
			(type == GL_FRAGMENT_SHADER) ? "fragment" : "vertex",
			&err_msg[0]);
		return 0;
	}
	return sid;
}
//the interface may change
static
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

/**
 *  @brief load shaders and create a program
 */
int
ShaderMan::loadShaders(const char *vshader , const char *fshader)
{
	GLuint vs, fs, p;
	if (!(vs = load_shader(vshader, GL_VERTEX_SHADER)))
		return -1;
	shaders.push_back(vs);
	if (!(fs = load_shader(fshader, GL_FRAGMENT_SHADER)))
		return -1;
	shaders.push_back(fs);
	if (!(p = load_shader_program(&shaders[0], 2)))
		return -1;
	pid = p;
	return 0;
}


ShaderMan::ShaderMan(const char *vshader, const char *fshader)
{
	loadShaders(vshader, fshader);
}
ShaderMan::~ShaderMan(void)
{
	glDeleteProgram(pid);
}

/* I should setup the texture before loading them */

bool
TextureMan::activeTexture(const char *name)
{
	GLuint texture;

	if (actived_texture >= 31 || !(texture = textures[name]))
		return false;
	glActiveTexture(GL_TEXTURE0+actived_texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	actived_texture+=1;
	return true;
}

bool
TextureMan::loadTexture(const char *img_fname, const char *ind)
{
	GLuint texture_handler;
	cv::Mat img;
	std::string new_img(img_fname);
	img = cv::imread(img_fname);
	if (!img.data)
		return false;
	glGenTextures(1, &texture_handler);
	glBindTexture(GL_TEXTURE_2D, texture_handler);
	//you will get a image that is upside down actually, maybe you need to flip the image before applying
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//and, don't forget add image to textures
	textures[ind] = texture_handler;
	curr_texture+= 1;
	return true;
}
