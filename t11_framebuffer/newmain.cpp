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

//#include <model.hpp>
#include "model.hpp"
#include "fbobj.hpp"
//#include "controls.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;

int main(int argc, char **argv)
{
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = tutorial_init(width, height);
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);

	ShaderMan postprocess("quadvs.glsl", "quadfs.glsl");
	glUseProgram(postprocess.getPid());
	FBobject fbobj(width, height);
	
	
	ShaderMan container("vs.glsl", "fs.glsl");
	glUseProgram(container.getPid());
	GLuint prog_id = container.getPid();
	TT::Model model(argv[1]);

	
	//create the depth and stencil buffer use renderbuffer
//	GLuint rbo;
//	glGenRenderbuffers(1, &rbo);
//	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	GLfloat theta = 0.0f;
	do {
		glfwPollEvents();

		fbobj.reffbo();
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(prog_id);

		GLint matAmbientLoc  = glGetUniformLocation(prog_id, "light.ambient");
		GLint matDiffuseLoc  = glGetUniformLocation(prog_id, "light.diffuse");
		GLint matSpecularLoc = glGetUniformLocation(prog_id, "light.specular");
	
		glUniform1f(matAmbientLoc,  0.3f);
		glUniform1f(matDiffuseLoc,  0.5f);
		glUniform1f(matSpecularLoc, 0.5f);
		
		theta += 0.01f;

		glm::mat4 m = glm::mat4(0.01f);
		glm::mat4 v = unity_like_get_camera_mat();
		glm::mat4 p = glm::perspective(glm::radians(90.0f),
					       (float)width / (float)height,
					       0.1f, 100.f);
		glm::mat4 mvp = p * v * m;
		

		glm::vec3 light_pos(cos(theta), 5.0f, -sin(theta));

		glUniform3f(glGetUniformLocation(prog_id, "viewPos"), 4.0f, 3.0f, 3.0f);
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		//std::cout << glGetUniformLocation(prog_id, "MVP") << std::endl;
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "model"), 1, GL_FALSE, &m[0][0]);
		//light's other attributes are setted in other places
		glUniform3f(glGetUniformLocation(prog_id, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
		
//		fbobj.unreffbo();
//		glDisable(GL_DEPTH_TEST);
		model.draw(prog_id);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" <<  std::endl;

//		fbobj.reffbo();
		fbobj.unreffbo();
//		glDisable(GL_DEPTH_TEST);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fbobj.drawfbo(postprocess.getPid());
		glfwSwapBuffers(window);

		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	//disable attributes
	glfwTerminate();
}
