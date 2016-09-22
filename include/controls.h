#ifndef CONTROLS_H
#define CONTROLS_H
#include <glm/glm.hpp>


void arcball_rotate(double xpos_prev, double ypos_prev,
		    double xpos, double ypos,
		    glm::mat4& rotation_mat);


#endif
