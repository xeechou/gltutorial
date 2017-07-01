#ifndef FBOBJ
#define FBOBJ
#include <GL/glew.h>

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
