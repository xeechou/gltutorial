#include <assert.h>
#include <stdio.h>
#include <tuple>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32

#include <GL/glfw3.h>
#endif

#include <context.hpp>
#include <shaderman.h>
#include <operations.hpp>
#include <controls.h>

void
context_winSizeChange(GLFWwindow *win, int width, int height)
{
	context *ctxt = (context *)glfwGetWindowUserPointer(win);
	ctxt->height = height;
	ctxt->width  = width;
	if (ctxt->intrinsic.fov > 0)
		ctxt->intrinsic.intrinsicMat = glm::perspective(ctxt->intrinsic.fov, (float)width/(float)height,
							      ctxt->intrinsic.near, ctxt->intrinsic.far);
	else
		ctxt->intrinsic.intrinsicMat = glm::ortho(0.0f, (float)width, (float)height, 0.0f,
							ctxt->intrinsic.near, ctxt->intrinsic.far);
	glViewport(0, 0, width, height);
}


DrawObj::DrawObj()
{
	this->ctxt = NULL;
}
DrawObj::DrawObj(GLuint shader)
{
	this->prog = shader;
}


//problem is, this is the boring code, nobody want to use it
//you will have to setup your own window callback function
context::context(int width, int height, const char *winname)
{
	_win = NULL;

	if (!glfwInit()) {
		fprintf(stderr, "Error: glfw init failed!\n");
		return;
	}
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	this->width = width;
	this->height = height;
	//create windows
	_win = glfwCreateWindow(width, height, winname, NULL, NULL);

	if (!_win) {
		fprintf(stderr, "man, no windows???\n");
		glfwTerminate();
		return;
	}
	//time to set userData
	glfwSetWindowUserPointer(_win, this);
	glfwSetWindowSizeCallback(_win, context_winSizeChange);

	glfwMakeContextCurrent(_win);
	//TODO: replace glew with glad
	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return;
	}
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	fprintf(stderr,"Renderer: %s\n", renderer);
	fprintf(stderr, "OpenGL version supported %s\n", version);
	glViewport(0, 0, width, height);
	std::cerr << this->forward_msg_que.size() << std::endl;
	this->setCameraPerspective(glm::radians(45.0f));
}

context::~context()
{
	glfwDestroyWindow(_win);
	glfwTerminate();
}

void
context::append_drawObj(DrawObj *dobj)
{
	dobj->set_context(this);
	this->drawobjs.push_back(dobj);
	dobj->setPosinContext(this->drawobjs.size()-1);
};


void
context::sendMsg(const DrawObj& d, const msg_t msg)
{
	int indx = d.getPosinContext();
	this->forward_msg_que.push(std::make_pair(indx+1, msg));
}

const msg_t
context::retriveMsg(const DrawObj& d)
{
	int indx;
	msg_t msg;
	std::tie(indx, msg) = this->forward_msg_que.front();
	if (indx == d.getPosinContext())
		this->forward_msg_que.pop();
	return msg;
}

const glm::vec2
context::retriveWinsize() const
{
	return glm::vec2(this->width, this->height);
}


int
context::init(void)
{
//	this->_forward_msg_que = decltype(this->_forward_msg_que)();
	for (unsigned int i = 0; i < drawobjs.size(); i++) {
		glUseProgram(drawobjs[i]->program());
		drawobjs[i]->init_setup();
	}
	assert(this->forward_msg_que.empty());
//	decltype(this->_forward_msg_que) swap_queue;
//	std::swap(this->_forward_msg_que, swap_queue);
	return 0;
}

int
context::run()
{
	do {
		//here comes the problem, should I finish all the setup before drawing or not?
		for (unsigned int i = 0; i < drawobjs.size(); i++) {
			glUseProgram(drawobjs[i]->program());
			drawobjs[i]->itr_setup();
		}
		glUseProgram(0);
		for (unsigned int i = 0; i < drawobjs.size(); i++) {
			//weird enough, glUseProgram caused problme for not drawing anything.
			glUseProgram(drawobjs[i]->program());
			drawobjs[i]->itr_draw();
		}
		assert(this->forward_msg_que.empty());
		glfwPollEvents();
		glfwSwapBuffers(_win);
	} while (glfwGetKey(_win, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		 !glfwWindowShouldClose(_win));
	return 0;
}



void
context::attachArcBallCamera(const float fov, const glm::vec3& pos, const glm::vec3& lookat)
{
	glfwSetCursorPosCallback(this->_win, unity_like_arcball_cursor);
	glfwSetScrollCallback(this->_win, unity_like_arcball_scroll);
//	glfwSetKeyCallback(this->_win, NULL)
	this->extrinsic.v = unity_like_get_camera_mat;
	this->extrinsic.p = unity_like_get_view_pos;
	this->setCameraPerspective(fov);
}

void
context::attachCamera(CameraInterface iface)
{
	this->extrinsic = iface;
	glfwSetCursorPosCallback(this->_win, iface.c);
	glfwSetScrollCallback(this->_win, iface.s);
	glfwSetKeyCallback(this->_win, iface.k);
}

void
context::setCameraPerspective(const float f, const float n, const float fr)
{
	this->intrinsic.fov = f;
	this->intrinsic.near = n;
	this->intrinsic.far = fr;
	this->intrinsic.intrinsicMat = glm::perspective(f, (float)this->width/(float)this->height, n, fr);
}

void
context::setCameraOrthognal(const float n, const float f)
{
	this->intrinsic.fov = -1.0f;
	this->intrinsic.near = n;
	this->intrinsic.far = f;
	this->intrinsic.intrinsicMat = glm::ortho(0.0f, (float)this->width, (float)this->height, 0.0f, n, f);
}

/*
Camera::Camera(const context* c, const float fov,
	       const glm::vec3& pos, const glm::vec3& lookat,
	       const float nearplane, const float farplane)
{
	this->ctxt = c;
	this->camera_pos = pos;
	this->look_axies = lookat -pos;
	this->fov = fov;
	this->np = nearplane;
	this->fp = farplane;

}

Camera::Camera(const context *ctxt,
	       const glm::vec3& pos, const glm::vec3& lookat,
	       const float nearplane, const float farplane)
{
	this->camera_pos = pos;
	this->look_axies = lookat - pos;
	this->fov = 0.0;
	this->np = nearplane;
	this->fp = farplane;
}

const glm::mat4
Camera::pvMat(void) const
{
	glm::mat4 pmat, vmat;
	float width = this->ctxt->getWidth();
	float height = this->ctxt->getHeight();
	if (this->fov != 0.0)
		pmat = glm::perspective(glm::radians(this->fov), width/height, this->np, this->fp);
	else
		pmat = glm::ortho(0.0f, width, height, 0.0f, this->np, this->fp);
	vmat = glm::lookAt(this->camera_pos, this->camera_pos + this->look_axies, glm::vec3(0,1,0));
//	print_glmMat4(vmat);
//	print_glmMat4(pmat);
	return pmat * vmat;
}
*/
