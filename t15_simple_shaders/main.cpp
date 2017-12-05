#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
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
#include <fbobj.hpp>
#include <context.hpp>
#include <collections/shaders.hpp>
#include <data.hpp>


const unsigned int width = 1024;
const unsigned int height = 1024;



class planeOBJ : public DrawObj {
private:
	ShaderMan _shader_program;
	GLuint _vao, _vbo;
	int time;
public:
	planeOBJ(int param);
	int init_setup(void) override;
	int itr_setup(void) override;
	int itr_draw(void) override;
};

planeOBJ::planeOBJ(int param)
{
	this->time = 0;
}

int
planeOBJ::init_setup(void)
{
	glGenVertexArrays(1, &_vao);
	glBindVertexArray(_vao);
	glGenBuffers(1, &_vbo);
	bindQUAD(_vao, _vbo);
	return 0;
}

int
planeOBJ::itr_setup(void)
{
	//we can increase the time of drawing
	return 0;
	this->time += 0.01;
}

int
planeOBJ::itr_draw(void)
{
	drawQUAD(_vao, _vbo);
}

int main(int argc, char **argv)
{
	context cont(width, height, "window");
	GLFWwindow *window = cont.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);

//	ShaderMan cubeShader("vs.glsl", "fs.glsl");
//	ShaderMan shadowShader("lightvs.glsl", "lightfs.glsl");
	cont.init();
	cont.run();
}
