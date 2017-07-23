#include <iostream>
#include <assert.h>
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../include/controls.h"

void arcball_rotate(double xpos_prev, double ypos_prev,
		    double xpos, double ypos,
		    glm::mat4& rotation_mat)
{
	if ((xpos * xpos + ypos * ypos) > 0.25)
		return;
	if ((xpos_prev * xpos_prev + ypos_prev * ypos_prev) > 0.25)
		return;
	//looks like we only deal with the front face of the sphere
	
	double zpos_prev = sqrt(0.25 - xpos_prev * xpos_prev - ypos_prev * ypos_prev);
	double zpos = sqrt(0.25 - xpos * xpos - ypos * ypos);
	//we should we original camear position, in spherical form.
	
	//alright, now we setup the camera position, but we want to deal with the relative system.
	//I just need to figure out the angle
	glm::vec3 prev_pos = glm::vec3(xpos_prev, ypos_prev, zpos_prev);
	glm::vec3 curr_pos = glm::vec3(xpos, ypos, zpos);
	glm::vec3 rotation_axis = glm::cross(prev_pos, curr_pos);
	float angle = acos(fmin(1.0, glm::dot(glm::normalize(prev_pos), glm::normalize(curr_pos) )) );
//	std::cout << "(" << prev_pos[0] << "," << prev_pos[1] << "," << prev_pos[2] << ")" << " and ";
//	std::cout << "(" << curr_pos[0] << "," << curr_pos[1] << "," << curr_pos[2] << ")" << std::endl;
//	std::cout << angle / M_PI << std::endl;
	rotation_mat = glm::rotate(rotation_mat, angle, glm::normalize(rotation_axis));
}


void
glfw2gl_coord(GLFWwindow *win, double glfw_xpos, double glfw_ypos,
	      double *glxpos, double *glypos)
{
	int width, height;
	glfwGetWindowSize(win, &width, &height);
	*glxpos = (glfw_xpos - width / 2.0) / width;
	*glypos = (height / 2.0 - glfw_ypos) / height;
}
	

class UnityArcBall {
	bool _inuse;
//	double _xpos_prev, _ypos_prev;
	glm::vec3 _camera_pos;
	glm::vec3 _lookat_pos;
	glm::mat4 _view_mat;
	
public:
	UnityArcBall(const glm::vec3& init_camera_pos, const glm::vec3& lookat) : _camera_pos(init_camera_pos), _lookat_pos(lookat) {
		_inuse = false;
	}
	//when you hold the right key.
	void arcball_rotate(double xpos, double ypos, double pxpos, double pypos);
	void arcball_translate(double xpos, double ypos, double xpos_prev, double ypos_prev);	
	void arcball_scale(double length);
	
	glm::vec3 getViewPos(void);
	glm::mat4 getViewMat(void);
};
static UnityArcBall ARCBALL(glm::vec3(4.0f, 3.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f));
void
UnityArcBall::arcball_rotate(double xpos, double ypos, double xpos_prev, double ypos_prev)
{
	if (_inuse)
		return;
	_inuse = true;

	if (xpos_prev == xpos && ypos_prev == ypos) {
		_inuse = false;
		return;
	}
	if ((xpos * xpos + ypos * ypos) > 0.25) {
		_inuse = false;
		return;
	}
	if ((xpos_prev * xpos_prev + ypos_prev * ypos_prev) > 0.25) {
		_inuse = false;
		return;
	}
	//first let's debugging this
	//std::cout << "xpos: " << xpos << ", ypos: " << ypos << ", xpos_prev: " << _xpos_prev << ", ypos_prev: " << _ypos_prev << std::endl;
	double zpos_prev = sqrt(0.25 - xpos_prev * xpos_prev - ypos_prev * ypos_prev);
	double zpos = sqrt(0.25 - xpos * xpos - ypos * ypos);
	
	glm::vec3 prev_dir = glm::vec3(xpos_prev, ypos_prev, zpos_prev);
	glm::vec3 curr_dir = glm::vec3(xpos, ypos, zpos);
	//as usual, get the rotation axies, the only difference is that we don't look at origin anymore
	glm::vec3 rotation_axis = glm::cross(prev_dir, curr_dir);
	float angle = acos(fmin(1.0, glm::dot(glm::normalize(prev_dir), glm::normalize(curr_dir) )) );
	
//	std::cout <<"angle: " << angle << std::endl;
	//we need to come up with new camera_position.
	glm::vec4 new_camera_pos = glm::rotate(angle, glm::normalize(rotation_axis)) *
		glm::vec4(_camera_pos - _lookat_pos, 1.0f) + glm::vec4(_lookat_pos, 1.0f);
	_camera_pos = glm::vec3(new_camera_pos);

	_inuse = false;
	return;
}

void
UnityArcBall::arcball_translate(double xpos, double ypos, double xpos_prev, double ypos_prev)
{
	glm::vec3 distance = glm::vec3(xpos - xpos_prev, ypos - ypos_prev, 0.0f);
	_camera_pos -= distance;
	_lookat_pos -= distance;
}
void UnityArcBall::arcball_scale(double length)
{
	if (_inuse)
		return;
	_inuse = true;
	glm::vec3 direction = _camera_pos - _lookat_pos;
	float norm = glm::length(direction);
	float new_norm = fmax(0.1f, norm+length);
	_camera_pos = glm::normalize(direction) * new_norm + _lookat_pos;
	_inuse = false;
}


glm::mat4
UnityArcBall::getViewMat(void)
{
	return glm::lookAt(_camera_pos, _lookat_pos, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3
UnityArcBall::getViewPos(void)
{
	return _camera_pos;
}

/** call backs */
void unity_like_arcball_cursor(GLFWwindow *win, double xpos, double ypos)
{
	static double xprev = 0.0f, yprev = 0.0f;
	static int lstate_prev = GLFW_RELEASE;
	static int rstate_prev = GLFW_RELEASE;
	static int mstate_prev = GLFW_RELEASE;
	(void)lstate_prev;
	(void)rstate_prev;
	(void)mstate_prev;
	
	glfw2gl_coord(win, xpos, ypos, &xpos, &ypos);
	int lstate = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);
	int rstate = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT);	
	int mstate = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE);
	if (lstate == GLFW_PRESS) {
		ARCBALL.arcball_translate(xpos, ypos, xprev, yprev);
	} else if (rstate == GLFW_PRESS) {
		ARCBALL.arcball_rotate(xpos, ypos, xprev, yprev);
	} else if (mstate == GLFW_PRESS) {
	}
	//update
	xprev = xpos, yprev = ypos;
	rstate_prev = rstate;
	lstate_prev = lstate;
	mstate_prev = mstate;

}


void unity_like_arcball_scroll(GLFWwindow *win, double xoffset, double yoffset)
{
	//xoffset is useless, only use yoffset
	ARCBALL.arcball_scale(0.1 * yoffset);
}


glm::mat4 unity_like_get_camera_mat(void)
{
	return ARCBALL.getViewMat();
}

glm::vec3 unity_like_get_view_pos(void)
{
	return ARCBALL.getViewPos();
}

