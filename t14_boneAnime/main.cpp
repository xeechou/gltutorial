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
#include "shadow.hpp"


const unsigned int width = 1024;
const unsigned int height = 1024;

//series of keyframes.

class Animator {
	//animator is in charge of switching between a few of animations
public:
	typedef std::pair<glm::mat4, std::vector<Bone> > instance_t;
private:
	//it contains the animated model, 
	const Model* model;
	//we will uses the instance from the models.
	std::vector<glm::mat4> instance_mats;
	//the bone of that reference, maybe we don't need to copy that
	std::map<std::string, Bone> bones; //we may doesnt have it though	
//	std::vector< std::vector<Bone> > bones;
	//and a reference to an animation
	float animation_time;

	//we may just keep the reference
	std::vector<Animation> animations;
public:

	Animator();
	/**
	 * @brief convinient constructor if you only have one model and one instance
	 */
	Animator(const Model* model);
	void setModel(const Model* model);
	void doAnimation();//do a new animation.
	void update();
//	void loadAnmiations(aiScene *scene);
};

//okay, forget about that shadertexture

class staticOBJ : DrawObj {
	//one object or instanced
private:
	std::shared_ptr<Model> model;
	glm::vec3 lightdir;
	glm::mat4 mvp;
	ShaderMan shader_program;
public:
	staticOBJ(void);
	//make it static, call before init_setup
	//Don't delete it.
	void setModel(std::shared_ptr<Model> model);
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

	shadowMap shadow;
	AfterShadow cubes;
	
	Model charactor(argv[1], Model::Parameter::LOAD_BONE | Model::Parameter::LOAD_ANIM);

	//the general shader, 
	
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
	this->shader_program.loadProgramFromString(vs_source, vs_source);
}


void
staticOBJ::setModel(std::shared_ptr<Model> model)
{
	this->model = model;
}

void
staticOBJ::setLight(glm::vec3 origin)
{
	this->lightdir = origin;
}

int

staticOBJ::init_setup()
{
	//basically you need to setup the uniforms and stuff
}


Animator::Animator(const Model* model)
{
	this->setModel(model);
}

void
Animator::setModel(const Model *model)
{
	this->model = model;
	//only one instance
	if (!model->getNinstances()) {
		//extract one bone
		instance_mats.push_back(glm::mat4(1.0f));
	}
	for (int i = 0; i < model->getNinstances(); i++) {
		//extract bones from the 
	}
}
