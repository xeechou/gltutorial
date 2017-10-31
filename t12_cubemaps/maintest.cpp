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
#include <collections/drawobjs.hpp>


const unsigned int width = 1024;
const unsigned int height = 1024;


namespace fs=boost::filesystem;

class nanoobj : public DrawObj {
private:
	glm::mat4 m, v, p;
	Model *model;
	float theta;
	GLuint cubeTex = -1;
	ShaderMan shader_program;
public:

	nanoobj(Model *m) : model(m) {

		const std::string vs_source =
#include "vs.glsl"
		;
		const std::string fs_source= 
#include "fs.glsl"
		;
		this->shader_program.loadProgramFromString(vs_source, fs_source);
		this->prog = shader_program.getPid();

		this->theta = 0.0f;
		this->m = glm::mat4(0.01f);
		this->v = unity_like_get_camera_mat();

		this->p = glm::perspective(glm::radians(90.0f),
					   (float)width / (float)height, 0.1f, 100.f);
//			count = 0;
	}
	void setCubeMapTex(GLuint cube_tex) {this->cubeTex = cube_tex;}
	int init_setup(void) override {
		this->model->bindShader(&shader_program);
		
		GLuint matAmbientLoc  = glGetUniformLocation(this->prog, "light.ambient");
		GLuint matDiffuseLoc  = glGetUniformLocation(this->prog, "light.diffuse");
		GLuint matSpecularLoc = glGetUniformLocation(this->prog, "light.specular");
		GLuint skyboxSampler  = glGetUniformLocation(this->prog, "skybox");


		glUniform1f(matAmbientLoc,  0.3f);
		glUniform1f(matDiffuseLoc,  0.5f);
		glUniform1f(matSpecularLoc, 0.5f);

		//for skybox
		glUniform1i(skyboxSampler, 0);
		return 0;
	}
	int itr_draw(void) override {
//		std::cout << "nano suit" << count++ << std::endl;
		glUseProgram(prog);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeTex);
//			glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		//you cannot do this
		model->drawProperty();
		return 0;
	}
	int itr_setup(void) override {
		this->theta += 0.1f;
		this->v = unity_like_get_camera_mat();
		glm::vec3 view_pos = glm::vec3(this->v[3]);
			
		glm::mat4 mvp = this->p * this->v * this->m;
		glm::vec3 light_pos(cos(this->theta), 5.0f, -sin(this->theta));

		//this is probably not right.
		glUniform3f(glGetUniformLocation(this->prog, "viewPos"), view_pos[0], view_pos[1], view_pos[2]);
		glUniformMatrix4fv(glGetUniformLocation(this->prog, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(this->prog, "model"), 1, GL_FALSE, &m[0][0]);
		//light's other attributes are setted in other places
		glUniform3f(glGetUniformLocation(this->prog, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
			
		return 0;
	}
		
};

int main(int argc, char **argv)
{
	context ctxt(1000, 1000, "window");
//there are keypress callback and cursor callback function.
	GLFWwindow *window = ctxt.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);


	
	CubeMap skybox;
	skybox.loadCubeMap(argv[2]);
	skybox.get_camera_mat = unity_like_get_camera_mat;
	
	Model model;
	model.addProperty("mesh", std::make_shared<Mesh1>());
	model.addProperty("instancing",
			  std::make_shared<Instancing>(10, Instancing::OPTION::square_instances, glm::vec3(0.1f)));
	model.load(argv[1]);

	nanoobj nsuit(&model);
	nsuit.setCubeMapTex(skybox.getCubeTex());
	
	ctxt.append_drawObj(&nsuit);
	ctxt.append_drawObj(&skybox);
	ctxt.init();
	ctxt.run();
}


