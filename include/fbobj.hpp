#ifndef FBOBJ
#define FBOBJ
#include <GL/glew.h>

//framebuffer for postprocess... I need to rename it
class FBobject {
private:
	GLuint vao, vbo;
	GLuint texture;
	GLuint fbo, rbo;

public:
	void drawfbo(GLuint prog);
	FBobject(int, int);
	~FBobject(void);
	void reffbo(void);
	void unreffbo();

};




#endif /* EOF */
