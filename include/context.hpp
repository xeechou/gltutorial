#ifndef GLCONTEXT_H
#define GLCONTEXT_H

#include <tuple>
#include <algorithm>
#include <vector>
#include <iostream>
#include <queue>
#include <utility>
#include <memory>
#include <thread>
#include <functional>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
class ShaderMan;

/**
 * @brief skeleton class for setup one type of data
 *
 * this thing should clean up the drawing process. There are some common
 * paradigms like framebuffers. Only one shader is limited on one DrawObj,
 * because otherwise you dont really know which shader to deal with.
 *
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
 *
 * Since most times it has one, we could setup here
 */
class DrawObj {
protected:
	int pos_in_context;
	GLuint prog;
	context *ctxt;
public:
	DrawObj();
	DrawObj(GLuint p);
	void set_shader(GLuint p);
	//we should have better shader interface next time
	GLuint program(void) const;
	//okay, we cannot really do anything on init_setup and itr_setup
	//okay, this thing only context should called it
	virtual int init_setup(void) = 0;
	//this get called first
	virtual int itr_setup(void) = 0;
	//then this get called
	virtual int itr_draw(void) = 0;
	//write your own function for loading data
	context * get_context(void) const;
	const int getPosinContext(void) const;
	void set_context(context *c);
	void setPosinContext(size_t pos);
};


//don't subclass this
class Camera {
private:
	glm::mat4 camera_mat;
	glm::vec3 camera_pos;
	glm::vec3 look_angle;

public:
	glm::Mat4 pv;
	enum type {
		persp = 0,
		orth  = 1
	};
	//good thing is when we do things here, we can update the view matrix very quickly
	Camera(const glm::vec3& pos, const glm::vec3& lookat,
	       const float fov, const float aspectRatio,
	       const float nearplane=1.0, const float farplane=100.0);

	Camera(const glm::vec3& pos, const glm::vec3& lookat,
	       const float left, const float right, const float bot, const float top,
	       const float nearplane=1.0, float farplane=100.0);
	const glm::mat4 pvMat(void) const;
	//setup the camera motion by
	//we can also add the input events to camera
};


class context {
protected:
	typedef void (*cb_t) (GLuint, void *data);
	std::vector<DrawObj *> drawobjs;
	GLFWwindow *_win;
	//local que is better defined, use this first
	std::queue<std::pair<int, msg_t> > forward_msg_que;
	std::queue<msg_t> _bcast_msg_que;
	int width, height;
	Camera cam;
public:
	context(int width=1000, int height=1000, const char *winname="window");
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
	glm::mat4 getCameraMat(void) const;
	void sendMsg(const DrawObj& d, const msg_t msg);
	const msg_t retriveMsg(const DrawObj& d);
	const glm::vec2 retriveWinsize() const;
	//friend declarations
	friend void context_winSizeChange(GLFWwindow *win, int width, int height);
	//solution to eliminate the setTexShader, getTexShader calls: message
	//queue. Basically I need a local message queue that send and retrieve
	//information at every draw iteration. It should be empty and the end.
};


/*
template<Derived>
struct FrameThread {
	//yep, you need to wait on this
	std::condition_variable cvar;
	std::mutex mtx;

	//there should be some
	void down() { //request a frame
		{
			std::lock_quard<std::mutex> lk(mtx);
			std::static_cast<Derived*>(this)->_down();
			//then you only uses derived classes, but again, you need to know the derived type at compile at
			//I think I will go with virtual functions again. Plus you need list or graph for that
		}
	}
	void up(void) {

	}
};
*/

class ThreadedContext : public context {
protected:
	std::shared_ptr<std::thread> timer;
	int _ms;
public:
	ThreadedContext(void);
	ThreadedContext(unsigned int ms);


};

//////inline functions
//DrawObj
inline context *
DrawObj::get_context() const
{
	return this->ctxt;
}
inline void
DrawObj::set_context(context *e)
{
	this->ctxt = e;
}
inline const int
DrawObj::getPosinContext() const
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
DrawObj::program() const
{
	return this->prog;
}

const context *
current_context(void);


#endif
