#ifndef GLCONTEXT_H
#define GLCONTEXT_H

#include <tuple>
#include <algorithm>
#include <vector>
#include <iostream>
#include <queue>
#include <utility>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include "types.hpp"

class context;
class DrawObj;


/**
 * @brief skeleton class for setup one type of data 
 *
 * this thing should clean up the drawing process. There are some common
 * paradigms like framebuffers.  
 * 
 * A message system design for this system. Two types of communications need to
 * be done: 1) local communication and 2) broadcasting.  Usually message queue
 * is a dispatching process, each message gets to the disired end when
 * dispatched. But this is too complicated, in our case, drawobjs are called in
 * a sequence. They are treated indivadually unless they are supposed to connect
 * together, for example, the shader that need to run twice. If we take this
 * sequencial assumption, the previous drawobj can only send message to next drawobj.
 *
 * The other type is the global message system. So you send the message to a
 * global queue. It get cleared out at the end of the iteration, and only one
 * guy can do it.
 */
class DrawObj {
	friend class context;
private:
	int pos_in_context;
	void set_context(context *c);
	void setPosinContext(size_t pos);

protected:
	GLuint prog;
	context *ctxt;
public:
	DrawObj();
	DrawObj(GLuint p);
	void set_shader(GLuint p);
	//we should have better shader interface next time
	GLuint program(void);
	//okay, this thing only context should called it
	virtual int init_setup(void) = 0;
	//this get called first
	virtual int itr_setup(void) = 0;
	//then this get called
	virtual int itr_draw(void) = 0;
	//write your own function for loading data
	context * get_context(void);
	const int getPosinContext(void);
};


class context {
protected:
	typedef void (*cb_t) (GLuint, void *data);
	std::vector<DrawObj *> drawobjs;
	GLFWwindow *_win;
	//local que is better defined, use this first
	std::queue<std::pair<int, msg_t> > forward_msg_que;
	std::queue<msg_t> _bcast_msg_que;
public:
	context(int width, int height, const char *winname);
	context();
	~context();
	GLFWwindow * getGLFWwindow() {return _win;}
	void append_drawObj(DrawObj *dobj);
	int init(void);
	int run();
	void (* init_pre_cb) (void *data);
	void (* init_post_cb) (void *data);
	void (* itr_pre_cb) (void *data);
	void (* itr_post_cb) (void *data);
	//lets test if this shit works
	void sendMsg(const DrawObj& d, const msg_t msg);
	const msg_t retriveMsg(const DrawObj& d);
	//solution to eliminate the setTexShader, getTexShader calls: message
	//queue. Basically I need a local message queue that send and retrieve
	//information at every draw iteration. It should be empty and the end.
};


//////inline functions
//DrawObj
inline context *
DrawObj::get_context()
{
	return this->ctxt;
}
inline void
DrawObj::set_context(context *e)
{
	this->ctxt = e;
}
inline const int
DrawObj::getPosinContext()
{
	return this->pos_in_context;
}
inline void
DrawObj::setPosinContext(size_t pos)
{
	this->pos_in_context = pos;
}
inline void
DrawObj::set_shader(GLuint p)
{
	this->prog = p;
}
inline GLuint
DrawObj::program()
{
	return this->prog;
}

//



#endif
