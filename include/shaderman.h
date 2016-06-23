#ifndef SHADERMAN_H
#define SHADERMAN_H
#include <vector>
#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif


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
};


#endif /* EOF */
