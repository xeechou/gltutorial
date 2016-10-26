#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <GL/glew.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils.h>
#include <shaderman.h>

//#include "controls.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;
using namespace glm;

GLfloat VERTICES[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

GLuint LoadShaders(const char *, const char *);
void update_position(GLFWwindow *window, glm::vec3& position, double deltaTime);
int main(void)
{
	if ( !glfwInit() ) {
		fprintf(stderr, "do you have glfw installed?\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3, shit
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	GLFWwindow *window = glfwCreateWindow(width, height, "window", NULL, NULL);
	if (!window) {
		fprintf(stderr, "main, no window???\n");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glewExperimental=true;

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	//Step 1: loading shaders
	ShaderMan shader_man("vs.glsl", "fs.glsl");
	GLuint prog_id = shader_man.getPid();
	
	//create vertex array and vertex buffer
	//generate buffers
	GLuint containerVAO, VBO;
	glGenVertexArrays(1, &containerVAO);
	glBindVertexArray(containerVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
	//load attributes, for layouts
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)0);
	glBindVertexArray(0);

	//second program
	ShaderMan light("vs.glsl", "lightfs.glsl");
	GLuint light_id = light.getPid();
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)0);
	glBindVertexArray(0);
	
	do {
		glfwPollEvents();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(prog_id);
		GLint obj_color_loc = glGetUniformLocation(prog_id, "objectColor");
		GLint light_color_loc = glGetUniformLocation(prog_id, "lightColor");
		glUniform3f(obj_color_loc, 1.0f, 0.5f, 0.31f);
		glUniform3f(light_color_loc,  1.0f, 1.0f, 1.0f); // Also set light's color (white)
		
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 View = glm::lookAt(glm::vec3(4,3,3),//position
					     glm::vec3(0,0,0),//where do you want to look at?
					     glm::vec3(0,1,0));//head is up
		glm::mat4 Model = glm::mat4(1.0f);
		glm::vec3 offset = glm::vec3(0.0f);
		GLuint MatrixID = glGetUniformLocation(prog_id, "mvp");
		glm::mat4 translation = glm::translate(offset);//the translation works, why does not the view matrix?
		glm::mat4 mvp = Projection * View * translation;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glBindVertexArray(containerVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		glUseProgram(light_id);
		glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
		glm::mat4 model;
		model = glm::translate(model, light_pos);
		model = glm::scale(model, glm::vec3(0.2f));
		glm::mat4 mvp2 = Projection * View * model;
		glUniformMatrix4fv(glGetUniformLocation(light_id, "mvp"), 1, GL_FALSE, &mvp2[0][0]);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glfwSwapBuffers(window);
		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	//disable attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &containerVAO);

}



void update_position(GLFWwindow *window, glm::vec3& offset, double deltaTime)
{
	float speed = 0.003f;
	float horizontalAngle = 3.14f;
	float verticalAngle = 0.0f;

	glm::vec3 right(1.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
		
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
//		std::cout << "up" << deltaTime << std::endl;
		offset += up * speed * (float)deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
//		std::cout << "down" << deltaTime << std::endl;
		offset -= up * speed * (float)deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
//		std::cout << "left" << deltaTime << std::endl;
		offset -= right * speed * (float)deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
//		std::cout << "right" << deltaTime << std::endl;
		offset += right * speed * (float)deltaTime;
	}
}


