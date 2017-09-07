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


const unsigned int width = 1024;
const unsigned int height = 1024;

class KeyFrame {
	//this vary from 0 to 1
	float timeStamp;
};

//now I have to write the animator class
//what I can do from here is instance with animator instead of models, but when you start to draw anything. you still calls model.draw?1
class Animation {
	//we have keyframes
	std::vector<KeyFrame> keyframes;
public:
//	Animation()
	
};

class Animator {
public:
	typedef std::pair<glm::mat4, std::vector<Bone> > instance_t;
private:
	//it contains the animated model, 
	const Model* model;
	//we will uses the instance from the models.
	std::vector<glm::mat4> instance_mats;
	//the bone of that reference, maybe we don't need to copy that
	std::vector< std::vector<Bone> > bones;
	//and a reference to an animation
	float animation_time;
	//get previous frames and last frames
public:

	Animator();
	/**
	 * @brief convinient constructor if you only have one model and one instance
	 */
	Animator(const Model* model);
	void setModel(const Model* model);
	void doAnimation();//do a new animation.
	void update();
};

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


int main(int argc, char **argv)
{
	context cont(width, height, "window");
	GLFWwindow *window = cont.getGLFWwindow();
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);
//	ShaderMan cubeShader("vs.glsl", "fs.glsl");
//	ShaderMan shadowShader("lightvs.glsl", "lightfs.glsl");
	
	Model charactor(argv[1], Model::Parameter::LOAD_BONE);
	
	cont.init();
	cont.run();
}
