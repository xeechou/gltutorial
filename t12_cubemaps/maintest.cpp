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

const unsigned int width = 1024;
const unsigned int height = 1024;


namespace fs=boost::filesystem;

class CubeMap : public DrawObj {
protected:
	GLuint cube_tex, vao, vbo;
	glm::mat4 p;
	
	unsigned int loadCubeMap(const fs::path& cubemaps);
public:
	CubeMap(GLuint pid, const fs::path& cubemaps) {
		this->prog = pid;
		this->cube_tex = this->loadCubeMap(cubemaps);
	}
	GLuint getCubeTex(void) const {return cube_tex;}
	int init_setup(void) override;
	//this get called first
	int itr_setup(void) override;
	//then this get called
	int itr_draw(void) override;
};




int main(int argc, char **argv)
{
	context ctxt(1000, 1000, "window");
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = ctxt.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);


	ShaderMan skybox_program("skyboxvs.glsl", "skyboxfs.glsl");
	CubeMap skybox(skybox_program.getPid(), argv[2]);
	
	ShaderMan container("vs.glsl", "fs.glsl");
	Model model(argv[1]);
	model.bindShader(&container);
	model.make_instances(10, Model::INIT_squares, glm::vec3(0.1f));
//	model.append_instance(glm::vec3(0.0f));
//	model.append_instance(glm::vec3(10.0f));
//	model.append_instance(glm::vec3(-10.0f));
//	model.append_instance(glm::vec3(20.0f));

//	model.make_instances(10, Model::INIT_random);
	model.pushIntances2GPU();

	class nanodobj : public DrawObj {
	private:
		glm::mat4 m, v, p;
		Model *model;
		float theta;
		GLuint cubeTex = -1;

	public:
		nanodobj(GLuint pid, Model *m) : DrawObj(pid), model(m) {
//			std::cout << "nano suit pid " << pid << std::endl;
			this->theta = 0.0f;
			this->m = glm::mat4(0.01f);
			this->v = unity_like_get_camera_mat();

			this->p = glm::perspective(glm::radians(90.0f),
						   (float)width / (float)height, 0.1f, 100.f);
//			count = 0;
		}
		void setCubeMapTex(GLuint cube_tex) {this->cubeTex = cube_tex;}
		int init_setup(void) override {
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
//			std::cout << "nano suit" << count++ << std::endl;
			glUseProgram(prog);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeTex);
//			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			//you cannot do this
			model->draw();
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
		
	} nsuit(container.getPid(), &model);
	nsuit.setCubeMapTex(skybox.getCubeTex());
	
	ctxt.append_drawObj(&nsuit);
	ctxt.append_drawObj(&skybox);
	ctxt.init();
	ctxt.run();
}


unsigned int
CubeMap::loadCubeMap(const fs::path& cubemaps)
{
	std::string faces[6];
	//load cubemaps information first, TODO remove this shitty code later
	{
		std::string content;
		read_file(cubemaps.string(), content);
		//read the six lines
		std::stringstream ss(content);
		for (int i = 0; i < 6; i++) {
			std::getline(ss, faces[i]);
		}
	}

	GLuint texture_id;
	glActiveTexture(GL_TEXTURE0);	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	for (int i = 0; i < 6; i++) {
		fs::path dir = cubemaps.parent_path();
		cv::Mat img = cv::imread((dir / fs::path(faces[i])).string());
		if (!img.data) {
			std::cerr << "where is your cubemap image" << std::endl;
			return -1;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			     0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return texture_id;
}

int CubeMap::init_setup(void)
{
	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3*sizeof(float)), (GLvoid *)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	//setup p here
	this->p = glm::perspective(glm::radians(90.0f),
				   (float)width / (float)height, 0.1f, 100.f);
	//setup texture

	//p is a constant, so we can use uniform here as well.
	return 0;
}

int
CubeMap::itr_setup()
{
	glm::mat4 v = glm::mat4(glm::mat3(unity_like_get_camera_mat()));
	glm::mat4 pv = this->p * v;
	glUniformMatrix4fv(glGetUniformLocation(this->prog, "pv"), 1, GL_FALSE, &pv[0][0]);

	return 0;
}

int
CubeMap::itr_draw()
{
	glDepthFunc(GL_LEQUAL);
//	glDisable(GL_DEPTH_TEST);
	
	glBindVertexArray(this->vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	GLuint cubetex_id = glGetUniformLocation(this->prog, "ctex");
	glUniform1i(cubetex_id, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
	return 0;
}
