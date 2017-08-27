#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <utils.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>

static float ASPECT_RATIO = 1.0;

int
read_file(std::string& fname, std::string &str)
{
//	assert(str);
	std::stringstream ss;
	std::ifstream f(fname, std::ios::in);
	if (!f.is_open())
		return -1;

	ss << f.rdbuf();
	str.assign(ss.str());
	f.close();
	return 0;
}

void tutorial_error_callback(int error, const char *description)
{
	fprintf(stderr, "Error Code: %d, %s\n", error, description);
}



static void
window_resize_callback(GLFWwindow *window, int width, int height)
{
	//we need to figure out the viewport
	int x, y, w, h;
	if ( ( (float)width / (float)height ) > ASPECT_RATIO) {
		w = height * ASPECT_RATIO; h = height;
		x = (width - w)/2; y = 0;
	} else {
		w = width; h = (int)( (float)width/ASPECT_RATIO );
		x = 0; h = (height -h) / 2;
	}
	glViewport(x, y, w, h);
}

GLFWwindow* tutorial_init(int width, int height,
			  void (*keyboard_callback)(GLFWwindow*, int, int, int, int),
			  void (*cursor_callback) (GLFWwindow *, double, double),
			  void (*scroll_callback) (GLFWwindow *, double, double))
{
	GLFWwindow *window;
       
	if (!glfwInit()) {
		fprintf(stderr, "Error: glfw init failed!\n");
		return NULL;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3, shit
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL

	//create windows
	window = glfwCreateWindow(width, height, "window", NULL, NULL);
	if (!window) {
		fprintf(stderr, "man, no windows???\n");
		glfwTerminate();
		return NULL;
	}
	ASPECT_RATIO = (float)width / (float) height;
	glfwSetErrorCallback(tutorial_error_callback);
	//for now we only have keyboard and mouse callback, let's try joystick later
	//cursor and keyboard is most prior callback you can have, afterwards...
	if (keyboard_callback)
		glfwSetKeyCallback(window, keyboard_callback);
	if (cursor_callback)
		glfwSetCursorPosCallback(window, cursor_callback);
	if (scroll_callback)
		glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowSizeCallback(window, window_resize_callback);
	glfwMakeContextCurrent(window);
	glewExperimental=true;

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		glfwTerminate();
		return NULL;
	}
//	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	fprintf(stderr,"Renderer: %s\n", renderer);
	fprintf(stderr, "OpenGL version supported %s\n", version);
	glViewport(0, 0, width, height);
	return window;

}


void tutorial_terminate(void)
{
	glfwTerminate();
}

void update_window(GLFWwindow* window)
{
	glfwPollEvents();
	glfwSwapBuffers(window);
}


//return the size of the buffer
