#include <stdexcept>
#include <sstream>
#include <iostream>
#include <GL/glew.h>
#include <stdio.h>
#include <types.hpp>
#include <shaderman.h>
#include <utils.h>


sprt_tex2d_t texture_types_supported[TEX_TYPE::TEX_NASSIMP_TYPE] = {
	{aiTextureType_AMBIENT, TEX_Ambient},
	{aiTextureType_DIFFUSE, TEX_Diffuse},
	{aiTextureType_NORMALS, TEX_Normal},
	{aiTextureType_SPECULAR, TEX_Specular},
};


static
GLuint load_shader(const char *fname, GLenum type)
{
	std::string shader_code;
	if (read_file(std::string(fname), shader_code)) {
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
		fprintf(stderr, "%s shader Compile info:  %s\n",
			(type == GL_FRAGMENT_SHADER) ? "fragment" : "vertex",
			&err_msg[0]);
		return 0;
	}
	return sid;
}

bool
ShaderMan::addTextureUniform(const std::string name, const TEX_TYPE type)
{
	glUseProgram(this->pid);
	//since the first texture is always texture0
	GLuint new_texture = this->texure_uniforms.size();
	this->texure_uniforms.push_back(std::make_pair(type, name));
	GLuint loc = glGetUniformLocation(this->pid, name.c_str());
	glUniform1i(loc, new_texture);
	this->uniforms[name] = loc;

	return true;
}

GLint
ShaderMan::getUniform(const std::string name) const
{
	auto it = this->uniforms.find(name);
	return (it != this->uniforms.end()) ? it->second : -1;
	
	return -1;
}

GLint
ShaderMan::getTexUniform(const TEX_TYPE type) const
{
	for (uint i = 0; i < this->texure_uniforms.size(); i++)
		if (this->texure_uniforms[i].first == type)
			return i;
	return -1;
}

//the interface may change

GLuint
ShaderMan::loadShaderProgram(GLuint *shaders, int len)
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
	if (!(p = ShaderMan::loadShaderProgram(&shaders[0], 2)))
		return -1;
	pid = p;
	return 0;
}

void
ShaderMan::loadProgramFromString(const std::string &vs, const std::string &fs)
{
	GLuint hvs, hfs, p;
	if (!(hvs = ShaderMan::createShaderFromString(vs, GL_VERTEX_SHADER)))
		goto err_loadShader;
	shaders.push_back(hvs);
	if (!(hfs = ShaderMan::createShaderFromString(fs, GL_FRAGMENT_SHADER)))
		goto err_loadShader;
	shaders.push_back(hfs);
	if (!(p = ShaderMan::loadShaderProgram(&shaders[0], 2)))
		goto err_loadShader;
	pid = p;
	return;
err_loadShader:
	throw std::runtime_error("failed to load shaders");
}

std::string
ShaderMan::getShaderName(GLenum shader)
{
	switch (shader) {
	case GL_VERTEX_SHADER:
		return "vertex shader";
		break;
	case GL_GEOMETRY_SHADER:
		return "geometry shader";
		break;
	case GL_FRAGMENT_SHADER:
		return "fragment shader";
		break;
	case GL_TESS_CONTROL_SHADER:
		return "tessellation control shader";
		break;
	case GL_TESS_EVALUATION_SHADER:
		return "tessellation evaluation shader";
		break;
	case GL_COMPUTE_SHADER:
		return "compute shader";
		break;
	default:
		return "unknown shader";
		break;
	}
}


GLuint
ShaderMan::createShaderFromString(const std::string& content, GLenum shadertype)
{
	int loglen;
	GLint result = GL_FALSE;
	GLuint sid = glCreateShader(shadertype);
	const char *shader_code_pointer = content.c_str();
	glShaderSource(sid, 1, &shader_code_pointer, NULL);
	glCompileShader(sid);
	
	//check compile statues
	glGetShaderiv(sid, GL_COMPILE_STATUS, &result);
	glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &loglen);
	if (result != GL_TRUE) {
		std::string err_msg;
		err_msg.resize(loglen+1, ' ');
//		std::vector<char> err_msg(loglen+1);
		glGetShaderInfoLog(sid, loglen, NULL, &err_msg[0]);
		std::cerr << ShaderMan::getShaderName(shadertype) << " Compile info: ";
		std::cerr << err_msg << std::endl;
		return 0;
	}
	return sid;
	
}


ShaderMan::ShaderMan(const char *vshader, const char *fshader)
{
	loadShaders(vshader, fshader);
}
ShaderMan::~ShaderMan(void)
{
	std::cerr << "called destructor" << std::endl;
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


//this is actually just 2DTexture
GLint
load2DTexture2GPU(const std::string fname)
{
	GLuint tid;

	glGenTextures(1, &tid);
	glBindTexture(GL_TEXTURE_2D, tid);
	cv::Mat img = cv::imread(fname, CV_LOAD_IMAGE_COLOR);
	if (!img.data) {
		std::cerr << "not valid image" << std::endl;
		return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Paramaters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tid;
}
