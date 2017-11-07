#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>

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
//#include "shadow.hpp"
//#include "animator.hpp"


const unsigned int width = 1024;
const unsigned int height = 1024;

//series of keyframes.

class staticOBJ : public DrawObj {
	//one object or instanced
private:
	std::vector<std::pair<std::string, std::shared_ptr<Model> > > draw_objects;
	std::shared_ptr<Model> drawobj;
	std::string file;
	glm::vec3 lightdir;
	ShaderMan shader_program;
public:
	staticOBJ(void);
	//make it static, call before init_setup
	//Don't delete it.
	void addModel(std::shared_ptr<Model> model, std::string toload="");
	void setLight(glm::vec3 origin);
	
	int init_setup(void) override;
	int itr_setup(void) override;
	int itr_draw(void) override;
};

int main(int argc, char **argv)
{
	context cont(width, height, "window");
	GLFWwindow *window = cont.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);

	std::shared_ptr<Model> small_guy = std::make_shared<Model>();
	small_guy->addProperty("mesh", std::make_shared<Mesh1>());
	small_guy->addProperty("material", std::make_shared<Material1>());
	small_guy->addProperty("joint", std::make_shared<Skeleton>(5));
	staticOBJ model;
	
	model.addModel(small_guy, std::string(argv[1]));
	model.setLight(glm::vec3(0, 100, 0));
	cont.append_drawObj(&model);
	cont.init();
	cont.run();
}

//I am supossing rightnow you should add shaders
staticOBJ::staticOBJ()
{
	std::string vs_source =
#include "vs.glsl"
		;
	std::string fs_source =
#include "fs.glsl"
		;
	//we can even forget about the specular for now
	this->shader_program.loadProgramFromString(vs_source, fs_source);
	this->prog = this->shader_program.getPid();
}

void
staticOBJ::addModel(std::shared_ptr<Model> model, std::string toload)
{
	this->drawobj = model;
	this->drawobj->bindShader(&this->shader_program);
	this->file = toload;
}


void
staticOBJ::setLight(glm::vec3 origin)
{
	this->lightdir = origin;
}

int
staticOBJ::init_setup()
{
	if (!this->file.empty()) {
		this->drawobj->load(this->file);
		this->drawobj->push2GPU();
	}
	
		
	this->shader_program.tex_uniforms.push_back(TEX_TYPE::TEX_Diffuse);
	GLuint lightAmbientLoc  = glGetUniformLocation(this->prog, "light.ambient");
	GLuint lightDiffuseLoc  = glGetUniformLocation(this->prog, "light.diffuse");
	GLuint lightSpecularLoc = glGetUniformLocation(this->prog, "light.specular");
	GLuint lightposLoc      = glGetUniformLocation(this->prog, "light.position");
	glUniform1f(lightAmbientLoc, 0.3f);
	glUniform1f(lightDiffuseLoc, 0.5f);
	glUniform1f(lightSpecularLoc, 0.5f);
	glUniform3f(lightposLoc, 0,100,3);
	//now setup the texture
	//okay, forget about the texture, they are done by bindTexture
	//also, we need to bind the texture to uniforms
	this->shader_program.addTextureUniform("diffuse", TEX_Diffuse);
	glUniform3f(glGetUniformLocation(this->prog, "viewPos"), 4.0,3.0,3.0);
	return 0;
}

int
staticOBJ::itr_setup()
{
	glm::vec2 wh = this->ctxt->retriveWinsize();
	glm::mat4 m = glm::mat4(1.0f);
	glm::mat4 v = unity_like_get_camera_mat();
	glm::mat4 p = glm::perspective(glm::radians(90.0f), (float)wh[0] / (float)wh[1],
				       0.1f, 100.0f);
	glm::mat4 mvp = p * v * m;
	//uniforms
	glUniformMatrix4fv(glGetUniformLocation(this->prog, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	return 0;
}

int
staticOBJ::itr_draw()
{
	//again, reminds you the purpose of the itr_draw
	glUseProgram(this->program());
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
//	glBindTexture(GL_TEXTURE2D, 0);
	//now we can draw
	this->drawobj->drawProperty();
	glUseProgram(0);
	return 0;
}
