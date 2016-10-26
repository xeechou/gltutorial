#ifndef SHADER_H
#define SHADER_H

#include <vector>
#include <string>
#include <string.h>
#include <GL/glew.h>

using namespace std;


/* the boring shader manger class */
class ShaderManager {
	std::vector<GLuint> shader_handlers;
	GLuint prog_id;
public:
	ShaderManger(void);
	~ShaderManger(void); /* destroy your program and related shaders */

	GLuint createShader(GLenum shaderType);
	GLuint createProgram(void); /*create a program use all your shaders */
	unsigned int countShaders(void); /* count all your shaders */
	void getShaders(GLuint *array); /* make sure you have sufficient memory of array */
};

unsigned int ShaderManager::countShaders(void)
{
	return this->shader_handlers.size();
}

void ShaderManager::getShaders(GLuint *array)
{
	memcpy(array, &(this->shader_handlers[0]), sizeof(GLuint) * this->shader_handlers.size());
}



#endif  /* SHADER_H */
