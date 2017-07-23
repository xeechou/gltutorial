#ifndef GLCONTEXT_H
#define GLCONTEXT_H

#include <tuple>
#include <algorithm>
#include <vector>
#include <iostream>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

/**
 * @brief skeleton class for setup one type of data 
 *
 * this thing should clean up the drawing process. There are some common paradigms like framebuffers. 
 */
class DrawObj {
protected:
	GLuint prog;
public:
	DrawObj() {}
	DrawObj(GLuint p) { this->prog = p;}
	void set_shader(GLuint p) {this->prog = p;}
	GLuint program(void) {return this->prog;}
	virtual int init_setup(void) = 0;
	//this get called first
	virtual int itr_setup(void) = 0;
	//then this get called
	virtual int itr_draw(void) = 0;
	//write your own function for loading data
};


class context {
	typedef void (*cb_t) (GLuint, void *data);
	std::vector<DrawObj *> drawobjs;
	GLFWwindow *_win;
public:
	context(int width, int height, const char *winname);
	context() : context(1000, 1000, "window") {}
	~context();
	GLFWwindow * getGLFWwindow() {return _win;}
	void append_drawObj(DrawObj *dobj) {
		this->drawobjs.push_back(dobj);
	};
	int init(void);
	int run();

	void (* init_pre_cb) (void *data);
	void (* init_post_cb) (void *data);
	void (* itr_pre_cb) (void *data);
	void (* itr_post_cb) (void *data);
};


#endif
