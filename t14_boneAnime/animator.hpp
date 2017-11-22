#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP


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


#endif
