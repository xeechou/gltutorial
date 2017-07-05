#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <boost/filesystem.hpp>

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

#include "context.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;


namespace fs=boost::filesystem;

struct nanosuit_runtime {
	
};


int main(int argc, char **argv)
{
	context context(1000, 1000, "window");
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = context.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);
	
	ShaderMan container("vs.glsl", "fs.glsl");
	glUseProgram(container.getPid());
	GLuint prog_id = container.getPid();
	Model model(argv[1]);

	class nanodobj : public DrawObj {
	private:
		glm::mat4 m, v, p;
		Model *model;
		float theta;
	public:
		nanodobj(GLuint pid, Model *m) : DrawObj(pid), model(m) {
			this->theta = 0.0f;
			this->m = glm::mat4(0.01f);
			this->v = unity_like_get_camera_mat();
			this->p = glm::perspective(glm::radians(90.0f),
						   (float)width / (float)height, 0.1f, 100.f);
		}
		int init_setup(void) override {
			GLuint matAmbientLoc  = glGetUniformLocation(this->prog, "light.ambient");
			GLuint matDiffuseLoc  = glGetUniformLocation(this->prog, "light.diffuse");
			GLuint matSpecularLoc = glGetUniformLocation(this->prog, "light.specular");

			glUniform1f(matAmbientLoc,  0.3f);
			glUniform1f(matDiffuseLoc,  0.5f);
			glUniform1f(matSpecularLoc, 0.5f);
			return 0;
		}
		int itr_draw(void) override {
			std::cerr << "here " << std::endl;
			//for this one, maybe no
			glUseProgram(prog);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			model->draw(prog);
			return 0;
		}
		int itr_setup(void) override {
			this->theta += 0.1f;
			this->v = unity_like_get_camera_mat();
			glm::mat4 mvp = this->p * this->v * this->m;
			glm::vec3 light_pos(cos(this->theta), 5.0f, -sin(this->theta));

			//this is probably not right.
			glUniform3f(glGetUniformLocation(this->prog, "viewPos"), 4.0f, 3.0f, 3.0f);
			glUniformMatrix4fv(glGetUniformLocation(this->prog, "MVP"), 1, GL_FALSE, &mvp[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(this->prog, "model"), 1, GL_FALSE, &m[0][0]);
			//light's other attributes are setted in other places
			glUniform3f(glGetUniformLocation(this->prog, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
		
			return 0;
		}
		
	} nsuit(container.getPid(), &model);
	
	context.append_drawObj(&nsuit);
	context.init();
	context.run();

}


