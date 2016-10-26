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
	double ratio = 0.5;//in this case it is really usesless
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

