#include <stdio.h>
#include <iostream>
#include <GL/glew.h>

#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils.h>

const unsigned int width = 1024;
const unsigned int height = 1024;

int main(void)
{
	if (!glfwInit()) {
		std::cout << "Error in init GLFW" << std::endl;
		return -1;
	}
	//WTF, Intel only support 3.3 even with mesa 3.3
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	GLFWwindow *window = glfwCreateWindow(width, height, "window", NULL, NULL);
	if (!window) {
		std::cout << "I cannot create window" << std::endl;
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

	//the gl code started
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID); 

	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	
	GLuint vertexBuffer;
	//generate 1 buffer
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW );

	
	//the projection matrix
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	//for an ortho camera:
	glm::mat4 View = glm::lookAt(glm::vec3(4,3,3),//position
				     glm::vec3(0,0,0),//where do you want to look at?
				     glm::vec3(0,1,0));//head is up
	glm::mat4 Model = glm::mat4(1.0f);//do nothing
	glm::mat4 mvp = Projection * View * Model;

	GLuint vsid = load_shader("vs.glsl", GL_VERTEX_SHADER);
	GLuint fsid = load_shader("fs.glsl", GL_FRAGMENT_SHADER);
	fprintf(stderr, "%d %d\n" ,vsid, fsid);
	GLuint shaders[] = {vsid, fsid};
	GLuint prog_id = load_shader_program(shaders, 2);
	GLuint MatrixID = glGetUniformLocation(prog_id, "MVP");

	do{
		glClear( GL_COLOR_BUFFER_BIT );
		glUseProgram(prog_id);

		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
			);
		
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDisableVertexAttribArray(0);
		// Draw nothing, see you in tutorial 2 !
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	glDeleteProgram(prog_id);

	
	//clean up
	glfwTerminate();
}
