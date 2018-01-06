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
	//It will be really nice if drawobj can be added through something like
	//scripting or graphic interface
protected:
	int pos_in_context;
	GLuint prog;
	context *ctxt;
public:
	DrawObj();
	DrawObj(GLuint p);
	virtual ~DrawObj(void) {}
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

typedef void (*CameraScrollFunc) (GLFWwindow *window, double xpos, double ypos);
typedef void (*CameraCursorFunc) (GLFWwindow* window, double xpos, double ypos);
typedef void (*CameraKeyFunc) (GLFWwindow *window, int key, int scancode, int action, int mods);
typedef glm::mat4 (*CameraVMatFunc) (void);
typedef glm::vec3 (*CameraPosFunc) (void);

class CameraInterface {
public:
	CameraCursorFunc c;
	CameraScrollFunc s;
	CameraKeyFunc k;
	CameraVMatFunc v;
	CameraPosFunc p;
};

class context {
protected:
	//I don't like this struct in here but the callback has to be
	struct camera {
		float fov;
		float near;
		float far;
		glm::mat4 intrinsicMat;
	};
	camera intrinsic;
	CameraInterface extrinsic;

	typedef void (*cb_t) (GLuint, void *data);
	std::vector<DrawObj *> drawobjs;
	GLFWwindow *_win;
	//local que is better defined, use this first
	std::queue<std::pair<int, msg_t> > forward_msg_que;
	std::queue<msg_t> _bcast_msg_que;
	int width, height;


public:
	context(int width=1000, int height=1000, const char *winname="window");
	~context();
	GLFWwindow * getGLFWwindow() {return _win;}
	void append_drawObj(DrawObj *dobj);
	int init(void);
	int run();
	//lets test if this shit works

	void sendMsg(const DrawObj& d, const msg_t msg);
	const msg_t retriveMsg(const DrawObj& d);
	const glm::vec2 retriveWinsize() const;
	const float getWidth(void) const;
	const float getHeight(void) const;
	//friend declarations
	friend void context_winSizeChange(GLFWwindow *win, int width, int height);
	//well, what we want is user can implment the interface so they can have a camera themselves
	void attachCamera(CameraInterface cam);
//		CameraPVMatFunc matfun, CameraKeyFunc keyfun, CameraCursorFunc posfun, CameraScrollFunc scrollfun);
	void attachArcBallCamera(const float fov, const glm::vec3& where, const glm::vec3& lookat=glm::vec3(0.0f));
	void attachFPSCamera(const float fov, const glm::vec3& where, const glm::vec3& lookat=glm::vec3(0.0f));
	void setCameraPerspective(const float fov, const float near=0.1f, const float far=100.0f);
	void setCameraOrthognal(const float near=0.1f, const float far=100.0f);
	const CameraInterface *getCameraExtrinsics(void) const { return &this->extrinsic; }
	const glm::mat4 getCameraMat(void) const {
		return this->intrinsic.intrinsicMat * extrinsic.v();
	}

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

inline const float
context::getWidth(void) const
{
	return this->width;
}
inline const float
context::getHeight(void) const
{
	return this->height;
}


#endif
