#ifndef SHADERMAN_H
#define SHADERMAN_H
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

class ShaderMan
{
	std::vector<GLuint>shaders;
	GLuint pid; //program id
public:
	int loadShaders(const char *, const char *);
	ShaderMan(void) {pid = 0;};
	ShaderMan(const char *vshader, const char *fshader);
	~ShaderMan();
	const GLuint getPid(void) {return pid;}
	void useProgram(void) {glUseProgram(pid);}
};



class TextureMan
{
	std::map<std::string, GLuint>textures;//is it a good idea to store textures by name? //
	size_t curr_texture;
	size_t actived_texture;
public:
	bool loadTexture(const char *fname, const char *ind_name);
	inline bool loadTexture(const char *fname) {
		return loadTexture(fname, fname);
	}
	bool activeTexture(const char *name);
	TextureMan(void) {curr_texture = 0; actived_texture = 0;}
	~TextureMan() {
		for (std::map<std::string, GLuint>::const_iterator it = textures.begin();
		     it != textures.end(); it++)
			glDeleteTextures(1, &(it->second));
	};
};





#endif
