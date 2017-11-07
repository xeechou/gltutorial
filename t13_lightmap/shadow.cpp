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
#include <collections/shaders.hpp>


#include "shadow.hpp"


//I think it is okay, since these two always come together
static glm::mat4 LP, LV;
static glm::vec3 LtP;

AfterShadow::AfterShadow(void)
{
	this->shader = std::make_shared<phongWithShadowMap>();
	this->prog = shader->getPid();
}

void AfterShadow::setCubeTex(GLuint tex_id)
{
	this->cubeTex = tex_id;
}

int AfterShadow::init_setup(void)
{
	//this is the light information, you can actually write one single draw
	//objects, but you need to change the shader program all the time. Well,
	//next implementation
	LP = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
	LV = glm::lookAt(glm::vec3(1.0, 3.0, 0.0), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));
	LtP = glm::vec3(1.0, 3.0, 0.0);
	
	for (unsigned int i = 0; i < cubes.size(); i++) {
		this->cubes[i]->bindShader(this->shader.get());		
		this->cubes[i]->push2GPU();
	}
	this->p = glm::perspective(glm::radians(90.0f), (float)1.0/(float)1.0, 1.0f, 100.0f);

	glm::mat4 lpv = LP * LV;
	glUniformMatrix4fv(glGetUniformLocation(this->prog, "lightMat"),
			   1, GL_FALSE, &lpv[0][0]);
		
	glUniform3f(glGetUniformLocation(this->prog, "lightPos"), LtP[0], LtP[1], LtP[2]);

	glUniform1i(glGetUniformLocation(this->prog, "cubetex"), 0);
	glUniform1i(glGetUniformLocation(this->prog, "shadowmap"), 1);

	//sendmsg and retriveMsg puts at the end of the operation
	this->depthTex = this->ctxt->retriveMsg(*this).u;
	return 0;
}

int
AfterShadow::itr_setup(void)
{
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

int
AfterShadow::itr_draw(void)
{
	glViewport(0, 0, 1024, 1024);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->cubeTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->depthTex);
	for (unsigned int i = 0; i < cubes.size(); i++)
		cubes[i]->drawProperty();
	return 0;
}


void
AfterShadow::append_model(Model *cube)
{
	this->cubes.push_back(cube);
}


shadowMap::shadowMap(void)
{
	this->shader = std::make_shared<shaderShadowMap>();
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

GLuint
shadowMap::getShadowTex() const
{
	return this->depthTex;
}

int
shadowMap::itr_setup() {return 0;}

int
shadowMap::itr_draw()
{
	glViewport(0,0, this->width, this->height);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	glUseProgram(this->prog);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	for (unsigned int i = 0; i < cubes.size(); i++) {
		cubes[i]->drawProperty(this->shader.get());
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return 0;
}
	


int shadowMap::init_setup(void)
{
	for (unsigned int i = 0; i < cubes.size(); i++) {
		//the problem is here, the mesh data get saved twice, somehow this caused no drawing problem
		//this->cubes[i]->push2GPU(Mesh::LOAD_POS);
		this->cubes[i]->bindShader(this->shader.get());
	}
	glm::mat4 lpv = LP * LV;
	glUniformMatrix4fv(glGetUniformLocation(this->prog, "lightMatrix"),
			   1, GL_FALSE, &lpv[0][0]);
	//setup framebuffer
	glBindTexture(GL_TEXTURE_2D, this->depthTex);
	//generate texture
	glGenFramebuffers(1, &this->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthTex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	//annouce the depthTexture to after shadow draw
	msg_t msg = {.u = this->depthTex};
	this->ctxt->sendMsg(*this, msg);

	return 0;
}

void
shadowMap::append_model(Model *cube)
{
	this->cubes.push_back(cube);
}
