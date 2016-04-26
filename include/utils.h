#ifndef UTILS_H
#define UTILS_H
#include <GL/glew.h>

GLuint load_shader(const char *fname, GLenum type);
GLuint load_shader_program(GLuint *, int);
#endif /* UTILS_H */
