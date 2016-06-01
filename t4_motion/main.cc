#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <GL/glew.h>

#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils.h>
#include "data.h"
#include "controls.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;
using namespace glm;

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

	//Step 1: loading shaders
	GLuint vsid = load_shader("vs.glsl", GL_VERTEX_SHADER);
	GLuint fsid = load_shader("fs.glsl", GL_FRAGMENT_SHADER);
	fprintf(stderr, "%d %d\n" ,vsid, fsid);
	GLuint shaders[] = {vsid, fsid};
	GLuint prog_id = load_shader_program(shaders, 2);
	glUseProgram(prog_id);

	//step 2: vertex data
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID); 

	GLuint vertexBuffer;
	//generate 1 buffer
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW );

	GLuint colorBuffer;
	glGenBuffers(1, &colorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);


	//load attributes
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0);
	//done loading attributes

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	//for an ortho camera:
	glm::mat4 View = glm::lookAt(glm::vec3(4,3,3),//position
				     glm::vec3(0,0,0),//where do you want to look at?
				     glm::vec3(0,1,0));//head is up
	//for now we just change the 
	glm::mat4 Model = glm::mat4(1.0f);
	glm::vec3 offset = glm::vec3(0.0f);

	GLuint MatrixID = glGetUniformLocation(prog_id, "MVP");
	
	double currentTime, deltaTime;
	double lastTime = glfwGetTime();
	
	do {
		glfwPollEvents();
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		update_position(window, offset, deltaTime);
		glm::mat4 translation = glm::translate(offset);

		glm::mat4 mvp = translation;
		glClear( GL_COLOR_BUFFER_BIT );
		
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		
		glDrawArrays(GL_TRIANGLES, 0, 12*3);
		glfwSwapBuffers(window);

		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	//disable attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDeleteProgram(prog_id);
}



void update_position(GLFWwindow *window, glm::vec3& offset, double deltaTime)
{
	float speed = 0.003f;
	float horizontalAngle = 3.14f;
	float verticalAngle = 0.0f;

	glm::vec3 right(1.0f, 0.0f, 0.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
		
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		std::cout << "up" << deltaTime << std::endl;
		offset += up * speed * (float)deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		std::cout << "down" << deltaTime << std::endl;
		offset -= up * speed * (float)deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		std::cout << "left" << deltaTime << std::endl;
		offset -= right * speed * (float)deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		std::cout << "right" << deltaTime << std::endl;
		offset += right * speed * (float)deltaTime;
	}
}
