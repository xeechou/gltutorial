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
	double _xpos_prev, _ypos_prev;
//	double _xpos_prev, _ypos_prev;
	glm::vec3 _camera_pos;
	glm::vec3 _lookat_pos;
	
	glm::mat4 _view_mat;

public:
	UnityArcBall(const glm::vec3& init_camera_pos, const glm::vec3& lookat) : _camera_pos(init_camera_pos), _lookat_pos(lookat) {
		_inuse = false;
		_xpos_prev = _ypos_prev = 0;
	}
	//when you hold the right key.
	void archball_rotate(GLFWwindow *win, double xpos, double ypos);
	glm::mat4 getViewMat(void);
};


void
UnityArcBall::archball_rotate(GLFWwindow *win, double xpos, double ypos)
{
	if (_inuse)
		return;
	_inuse = true;
	glfw2gl_coord(win, xpos, ypos, &xpos, &ypos);
	if (xpos == _xpos_prev && ypos == _ypos_prev)
		return;
	if ((xpos * xpos + ypos * ypos) > 0.25)
		return;
	if ((_xpos_prev * _xpos_prev + _ypos_prev * _ypos_prev) > 0.25)
		return;

	double zpos_prev = sqrt(0.25 - _xpos_prev * _xpos_prev - _ypos_prev * _ypos_prev);
	double zpos = sqrt(0.25 - xpos * xpos - ypos * ypos);
	
	glm::vec3 prev_dir = glm::vec3(_xpos_prev, _ypos_prev, zpos_prev) - _lookat_pos;
	glm::vec3 curr_dir = glm::vec3(xpos, ypos, zpos) - _lookat_pos;
	//as usual, get the rotation axies, the only difference is that we don't look at origin anymore
	glm::vec3 rotation_axis = glm::cross(prev_dir, curr_dir);
	float angle = acos(fmin(1.0, glm::dot(glm::normalize(prev_dir), glm::normalize(curr_dir) )) );

	//we need to come up with new camera_position.
	glm::vec4 new_camera_pos = glm::rotate(angle, glm::normalize(rotation_axis)) *
		glm::vec4(_camera_pos - _lookat_pos, 1.0f) + glm::vec4(_lookat_pos, 1.0f);
	_camera_pos = glm::vec3(new_camera_pos); //this should work.


	//finally, updating the _xpos_prev, and _ypos_prev
	_xpos_prev = xpos; _ypos_prev = ypos;
	_inuse = false;
	
}

static UnityArcBall ARCBALL(glm::vec3(4.0f, 3.0f, 3.0f), glm::vec3(1.0f, 1.0f, 1.0f));

glm::mat4
UnityArcBall::getViewMat(void)
{
	return glm::lookAt(_camera_pos, _lookat_pos, glm::vec3(0.0f, 1.0f, 0.0f));
}

void unity_like_arcball(GLFWwindow *win, double xpos, double ypos)
{

	static int lstate_prev = GLFW_RELEASE;
	static int rstate_prev = GLFW_RELEASE;
	static int mstate_prev = GLFW_RELEASE;

	int lstate = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);
	int rstate = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT);	
	int mstate = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE);
	if (lstate == GLFW_PRESS) {
		
	} else if (rstate == GLFW_PRESS) {
		ARCBALL.archball_rotate(win, xpos, ypos);
	} else if (mstate == GLFW_PRESS) {
		
	}
	rstate_prev = rstate;
	lstate_prev = lstate;
	mstate_prev = mstate;
}

glm::mat4 unity_like_get_camera_mat(void)
{
	return ARCBALL.getViewMat();
}
