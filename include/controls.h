#ifndef CONTROLS_H
#define CONTROLS_H
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#ifdef __cplusplus
#define _EXTERNC extern "C"
#else
#define _EXTERNC
#endif

//!
//! @brief converting glfw(same as opencv) image coordinates to opengl xy-coord system
void glfw2gl_coord(GLFWwindow * win, double glfw_xpos, double glfw_ypos,
			    double *gl_xpos,  double *gl_ypos);

void arcball_rotate(double xpos_prev, double ypos_prev,
		    double xpos, double ypos,
		    glm::mat4& rotation_mat);


//This is what they called singleton design pattern
void unity_like_arcball_cursor(GLFWwindow* window, double xpos, double ypos);
void unity_like_arcball_scroll(GLFWwindow* window, double xoffset, double yoffset);
//this is the wrapper for the unity camera
glm::mat4 unity_like_get_camera_mat(void);
glm::vec3 unity_like_get_view_pos(void);

glm::mat4 fps_get_camera_mat(void);
void fps_cursor(GLFWwindow *window, double xpos, double ypos);
void fps_keyboard(GLFWwindow *window, int key, int scancode, int action, int mods);

#endif
