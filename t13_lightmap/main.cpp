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

static glm::mat4 LP, LV;
static glm::vec3 LtP;

class AfterShadow : public DrawObj {
protected:
	std::vector<Model *> cubes;
	glm::mat4 p ;
	const ShaderMan *shader;
	//FBobject depth_frambuffer;
	GLuint depthTex, cubeTex;
public:
	AfterShadow(ShaderMan *shader) {
		this->shader = shader;
		this->prog = shader->getPid();
	}
	void setDepthTex(GLuint tex_id) {
		this->depthTex = tex_id;
	}
	void setCubeTex(GLuint tex_id) {
		this->cubeTex = tex_id;
	}
	int init_setup(void) override {
		for (unsigned int i = 0; i < cubes.size(); i++) {
			this->cubes[i]->push2GPU(Mesh::LOAD_POS | Mesh::LOAD_NORMAL | Mesh::LOAD_TEX);
			this->cubes[i]->bindShader(this->shader);
		}
		this->p = glm::perspective(glm::radians(90.0f), (float)1.0/(float)1.0, 1.0f, 100.0f);

		glm::mat4 lpv = LP * LV;
		glUniformMatrix4fv(glGetUniformLocation(this->prog, "lightMat"),
				   1, GL_FALSE, &lpv[0][0]);
//		std::cout << glGetUniformLocation(this->prog, "lightMat") << std::endl;
		
		glUniform3f(glGetUniformLocation(this->prog, "lightPos"), LtP[0], LtP[1], LtP[2]);
//		std::cerr << glm::to_string(lpv) << std::endl;

		glUniform1i(glGetUniformLocation(this->prog, "cubetex"), 0);
		glUniform1i(glGetUniformLocation(this->prog, "shadowmap"), 1);

		return 0;
	}
	int itr_setup(void) override {
		glm::mat4 v = unity_like_get_camera_mat();
		glm::mat4 pv = this->p * v;
		glm::vec3 view_pos = unity_like_get_view_pos();

//		std::cout << glGetUniformLocation(this->prog, "MVP") << std::endl;
		glUniformMatrix4fv(glGetUniformLocation(this->prog, "MVP"),
				   1, GL_FALSE, &pv[0][0]);
		glUniform3f(glGetUniformLocation(this->prog, "viewPos"),
			    view_pos[0], view_pos[1], view_pos[2]);
		return 0;
	}
	int itr_draw(void) override {
		glViewport(0, 0, 1024, 1024);
		glUseProgram(this->prog);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->cubeTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, this->depthTex);
		for (unsigned int i = 0; i < cubes.size(); i++)
			cubes[i]->draw();
		return 0;
	}
	void append_model(Model *cube) {
		this->cubes.push_back(cube);
	}
};

//this should go to a drawing paradgim
class shadowMap : public DrawObj {
protected:
	std::vector<Model *> cubes;
	const ShaderMan *shader;
	GLuint fbo, depthTex;
	size_t width, height;
public:
	shadowMap(ShaderMan *shader) {
		this->shader = shader;
		this->prog = shader->getPid();
		//put it here so we can render the 
		this->width = 1024; this->height = 1024;
		//generate framebuffer
		glGenTextures(1, &depthTex);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	GLuint getShadowTex() const { return this->depthTex; }

	int init_setup(void) override {
		for (unsigned int i = 0; i < cubes.size(); i++) {
			//the problem is here, the mesh data get saved twice, somehow this caused no drawing problem
			//this->cubes[i]->push2GPU(Mesh::LOAD_POS);
			this->cubes[i]->bindShader(this->shader);
		}
		//mysterious orthognal projection 
		glm::mat4 lpv = LP * LV;
		glUniformMatrix4fv(glGetUniformLocation(this->prog, "MVP"),
				   1, GL_FALSE, &lpv[0][0]);
		
		glBindTexture(GL_TEXTURE_2D, this->depthTex);
		//generate texture
		glGenFramebuffers(1, &this->fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthTex, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return 0;		
	}
	int itr_setup(void) override {
		return 0;
	}

	int itr_draw(void) override {
		glViewport(0,0, this->width, this->height);		
		glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
		glUseProgram(this->prog);
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		for (unsigned int i = 0; i < cubes.size(); i++)
			cubes[i]->draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return 0;
	}

	void append_model(Model *cube) {
		this->cubes.push_back(cube);
	}
	
};

int main(int argc, char **argv)
{
	context cont(width, height, "window");
	GLFWwindow *window = cont.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);
	GLuint cubeTex = loadTexture2GPU("../imgs/container.jpg");

	ShaderMan cubeShader("vs.glsl", "fs.glsl");
	ShaderMan shadowShader("svs.glsl", "sfs.glsl");
	shadowMap shadow(&shadowShader);
	AfterShadow cubes(&cubeShader);
	cubes.setDepthTex(shadow.getShadowTex());
	cubes.setCubeTex(cubeTex);

	LP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
	LV = glm::lookAt(glm::vec3(1.0, 3.0, 0.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
	LtP = glm::vec3(1.0, 3.0, 0.0);

	
	CubeModel cube;
//	cube.make_instances(100, Model::INIT_squares);
	cube.append_instance(glm::vec3(0.0f, 1.0f, 0.0f));
	cube.append_instance(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(10.0f, 1.0f, 10.0f));
	cube.append_instance(glm::vec3(-3.0f,  1.0f, 3.0f), glm::vec3(1.2f, 1.2f, 1.2f),
			     glm::quat(glm::vec3(90.0, 0.0, 0.0)));
	
	cubes.append_model(&cube);
	shadow.append_model(&cube);
	cont.append_drawObj(&shadow);
	cont.append_drawObj(&cubes);
	
	GLuint prog_id = cubeShader.getPid();
	GLfloat theta = 0.0f;

	cont.init();
	cont.run();
}
