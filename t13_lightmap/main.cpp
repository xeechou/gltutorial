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
#include <collections/geometry.hpp>
#include "shadow.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;

int main(int argc, char **argv)
{
	context cont(width, height, "window");
	GLFWwindow *window = cont.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);
	GLuint cubeTex = load2DTexture2GPU(argv[1]);

	shadowMap shadow;
	AfterShadow cubes;
	cubes.setCubeTex(cubeTex);
	
	CubeModel cube;
	RSTs instances;
	instances.addInstance(glm::vec3(0.0f, 1.0f, 0.0f));
	instances.addInstance(glm::vec3(0.0f, -1.0f, 0.0f), glm::quat(glm::vec3(0.0f)), glm::vec3(10.0f, 1.0f, 10.0f));
	instances.addInstance(glm::vec3(-3.0f,  1.0f, 3.0f),
			      glm::quat(glm::vec3(90.0, 0.0, 0.0)),
			      glm::vec3(1.2f, 1.2f, 1.2f));
	cube.addProperty("instancing", std::make_shared<Instancing>(instances));
	
	cubes.append_model(&cube);
	shadow.append_model(&cube);
	cont.append_drawObj(&shadow);
	cont.append_drawObj(&cubes);
	
	cont.init();
	cont.run();
}
