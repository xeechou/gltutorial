#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


//my headers
#include <utils.h>
#include <shaderman.h>
#include <controls.h>

#include <model.hpp>

//#include "controls.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;

glm::mat4 ROTATION = glm::mat4(1.0);
glm::mat4& get_rotation_mat(void) {
	return ROTATION;
}


//it do nothing for now
void tutorial_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glm::mat4 *model, *view, *projection;

	//we have a function for get/set current model, view, projection matrix.
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		
	} else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		
	} else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		
	} else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		
	}
	
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	//the first position is the middle
	static double lxpos_prev = 0.0;
	static double lypos_prev = 0.0;
	static double rxpos_prev = 0.0;
	static double rypos_prev = 0.0;
	static int lstate_prev = GLFW_RELEASE;
	static int rstate_prev = GLFW_RELEASE;

	//I am not sure, I need to
	//okay, the xpos and ypos is in opencv-Coordinates
	xpos = (xpos - width / 2.0) / width;
	ypos = (height/ 2.0 - ypos) / height;

	int lstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	int rstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (xpos == rxpos_prev && ypos == rypos_prev)
		return;
		
	if (lstate == GLFW_PRESS) {
		//moving_joint(window, lxpos_prev, lypos_prev, xpos, ypos);

	} else if (rstate == GLFW_PRESS) {
//		std::cout << xpos << " and " << ypos << std::endl;		
//		if (rstate_prev == GLFW_RELEASE) {
//			rxpos_prev = xpos - 0.000001; rypos_prev = ypos - 0.000001;
//		}
		arcball_rotate(rxpos_prev, rypos_prev, xpos, ypos, get_rotation_mat());
	}
	rstate_prev = rstate;
	lstate_prev = lstate;
	lxpos_prev = xpos; lypos_prev = ypos;
	rxpos_prev = xpos; rypos_prev = ypos;
}
		
		
int main(int argc, char **argv)
{
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = tutorial_init(width, height, NULL, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);
	//we have two object to draw
	ShaderMan container("vs.glsl", "fs.glsl");
	GLuint prog_id = container.getPid();
	glUseProgram(prog_id);
	
	Model nanosuit(argv[1]);
	std::cout << "Done loading models" << std::endl;
	std::cout << "light.diffuse is " << glGetUniformLocation(prog_id, "light.ambient") << std::endl;
	//the only thing that material doesn't change
	glUniform1f(glGetUniformLocation(prog_id, "material.shininess"), 32.0f);
	//here we are
	nanosuit.bindShader(&container);
	

	glUniform3f(glGetUniformLocation(prog_id, "objectColor"), 1.0f, 0.5f, 0.31f);
	glUniform3f(glGetUniformLocation(prog_id, "lightColor"), 1.0f, 1.0f, 1.0f);
	
	//light meterial
	GLint matAmbientLoc  = glGetUniformLocation(prog_id, "light.ambient");
	GLint matDiffuseLoc  = glGetUniformLocation(prog_id, "light.diffuse");
	GLint matSpecularLoc = glGetUniformLocation(prog_id, "light.specular");
	
	glUniform1f(matAmbientLoc,  0.3f);
	glUniform1f(matDiffuseLoc,  0.5f);
	glUniform1f(matSpecularLoc, 0.5f);

	
	ShaderMan light("lightvs.glsl", "lightfs.glsl");
	GLuint light_id = light.getPid();
	GLfloat theta = 0.0f;

	do {
		glfwPollEvents();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//for container, the uniform data does not change, so we can apply now
		glm::mat4 Model = glm::scale(glm::vec3(0.15f));
		//Model = glm::scale(Model, glm::vec3(0.1f));
//		glm::mat4 View  = glm::lookAt(glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0));
		glm::mat4 View  = unity_like_get_camera_mat();
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 mvp = Projection * View * Model;

		//light program
		
		glUseProgram(light_id);
		//compute the light pos at every step, the light is in 
		glm::vec3 light_pos(cos(theta), 1.0f, -sin(theta));
//		std::cout << "The light position is ["
//			  << light_pos[0] << ", "
//			  << light_pos[1] << ", "
//			  << light_pos[2] << "]" << std::endl;
		theta += 0.01f;

		glm::mat4 model = glm::translate(light_pos);
		model = glm::scale(model, glm::vec3(0.01f));
		glm::mat4 mvp2 = Projection * View * model;
		glUniformMatrix4fv(glGetUniformLocation(light_id, "mvp"), 1, GL_FALSE, &mvp2[0][0]);


		//for the container
		glUseProgram(prog_id);
		glUniform3f(glGetUniformLocation(prog_id, "viewPos"), 4.0f, 3.0f, 3.0f);
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		//std::cout << glGetUniformLocation(prog_id, "MVP") << std::endl;
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "model"), 1, GL_FALSE, &Model[0][0]);
		//light's other attributes are setted in other places
		glUniform3f(glGetUniformLocation(prog_id, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
//		std::cout << "(" << light_pos[0] << ", " << light_pos[1] << ", " << light_pos[2] << ")" << std::endl;
//		std::cout << glGetUniformLocation(prog_id, "light.position") << std::endl;

		nanosuit.draw();

		glfwSwapBuffers(window);
		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	//disable attributes
	glfwTerminate();
}
