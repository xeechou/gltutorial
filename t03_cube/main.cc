#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <GL/glew.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <shaderman.h>
#include <utils.h>

const unsigned int width = 1024;
const unsigned int height = 1024;
using namespace glm;

GLuint loadDDS(const char * imagepath);
GLuint LoadShaders(const char *, const char *);
GLuint load_texture(const char *);
int main(void)
{
	if ( !glfwInit() ) {
		fprintf(stderr, "do you have glfw installed?\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
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

	ShaderMan shader_man("vs.glsl", "fs.glsl");
	glUseProgram(shader_man.getPid());
	//the gl code started
	GLfloat vertices[] = {
		// Positions          // Colors           // Texture Coords
		0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Top Right
		0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Bottom Right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Bottom Left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // Top Left
	};
	GLuint indices[] = {  // Note that we start from 0!
		0, 1, 3, // First Triangle
		1, 2, 3  // Second Triangle
	};
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	// Color attribute
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	// TexCoord attribute
	glEnableVertexAttribArray(2);	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	//glUniform1i(glGetUniformLocation(shader_man.getPid(), "ourTexture"), 0);
	glBindVertexArray(0); // Unbind VAO
	
	//loading textures
	GLuint textures[2];
	//texture0
	glGenTextures(2, textures);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	cv::Mat image = cv::imread("../imgs/container.jpg");
	if (!image.data) {
		std::cout << "error loading textures" << std::endl;
		return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	//texture1
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	image = cv::imread("../imgs/awesomeface.png");
	if (!image.data) {
		std::cout << "error loading textures" << std::endl;
		return -1;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glUniform1i(glGetUniformLocation(shader_man.getPid(), "ourTexture0"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glUniform1i(glGetUniformLocation(shader_man.getPid(), "ourTexture1"), 1);


	glm::mat4 trans;
	do{
		trans = glm::rotate(trans, 0.10f, glm::vec3(0,0,1.0f));
		//trans = glm::scale(trans, glm::vec3(0.5,0.5,0.5));
		glClear( GL_COLOR_BUFFER_BIT );
		GLuint transformloc = glGetUniformLocation(shader_man.getPid(), "transform");
		glUniformMatrix4fv(transformloc, 1, GL_FALSE, &trans[0][0]);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// Draw nothing, see you in tutorial 2 !
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteTextures(2, textures);
	glfwTerminate();
}
