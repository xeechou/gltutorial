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
#include <fbobj.hpp>
#include <context.hpp>


const unsigned int width = 1024;
const unsigned int height = 1024;


//we will use instancing to replace this later
class cubesDraw : public DrawObj {
protected:
	std::vector<CubeModel *> cubes;
	glm::mat4 p ;
	const ShaderMan *shader;
public:
	cubesDraw(ShaderMan *shader) {
		this->prog = shader->getPid();
	}
	int init_setup(void) override {
		for (unsigned int i = 0; i < cubes.size(); i++) {
			this->cubes[i]->push2GPU(Mesh::LOAD_POS | Mesh::LOAD_NORMAL);
			this->cubes[i]->bindShader(this->shader);
		}
		this->p = glm::perspective(glm::radians(90.0f), (float)1.0/(float)1.0, 0.1f, 100.0f);		
		return 0;
	}
	int itr_setup(void) override {
		glm::mat4 v = unity_like_get_camera_mat();
		glm::mat4 pv = this->p * v;

		glUniformMatrix4fv(glGetUniformLocation(this->prog, "MVP"),
				   1, GL_FALSE, &pv[0][0]);
		return 0;
	}

	int itr_draw(void) override {
		glUseProgram(this->prog);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		for (unsigned int i = 0; i < cubes.size(); i++)
			cubes[i]->draw();
		return 0;
	}
};


int main(int argc, char **argv)
{
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = tutorial_init(width, height);
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);

	ShaderMan cubeShader("vs.glsl", "fs.glsl");

	CubeModel cube0;
	
	GLuint prog_id = cubeShader.getPid();
	GLfloat theta = 0.0f;
	do {
		glfwPollEvents();
		glfwSwapBuffers(window);
		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	glfwTerminate();
}
