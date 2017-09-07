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

//now I have to write the animator class
//what I can do from here is instance with animator instead of models, but when you start to draw anything. you still calls model.draw?1
class Animation {
	//Animation class is in charge of one animation sequence, so it has a
	//series of keyframes.
	class KeyFrame {
		//this vary from 0 to 1
		float timeStamp;
		glm::vec3 translation;
		glm::quat rotation;
		glm::vec3 scale;
	};
	//per bone. per frames
	std::vector< std::vector<KeyFrame> > keyframes;
	//we have times.

	//so what we are gonna do?
public:
	//it should have the ability to start the sequence, intenpolate between last keyframe and this
	//at every frame, we should calculate the bone sequence
	Animation();
	void allocateBones(int);
	void allocateFrames(int);
};

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
	void loadAnmiations(aiScene *scene);
};



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

void
Animator::loadAnmiations(aiScene *scene)
{
	this->animations.resize(scene->mNumAnimations);
	for (uint i = 0; i < scene->mNumAnimations; i++) {
		//it will allocate nbones.
		//you don't necessary have it.
		this->animations[i].allocateBones(this->bones.size());
		aiAnimation *anim = scene->mAnimations[i];
		size_t total_frames = anim->mTicksPerSecond * anim->mDuration;
		//now we need to get number of ticks
		for (uint j = 0; j > anim->mNumChannels; j++) {
			aiNodeAnim *bone_anim = anim->mChannels[j];
			//find the bone
			std::string name = bone_anim->mNodeName.C_Str();
			int ind = this->bones[name].getInd();
			this->animations[i].allocateFrames(total_frames);
			(void)bone_anim->mNumPositionKeys;
			(void)bone_anim->mNumRotationKeys;
			(void)bone_anim->mNumScalingKeys;
//			bone_anim->
				
		}
	}
}
