#ifndef DR_SHADOW_HPP
#define DR_SHADOW_HPP

#include <memory>

#include <utils.h>
#include <shaderman.h>
#include <controls.h>
#include <model.hpp>
#include <fbobj.hpp>
#include <context.hpp>
#include <collections/geometry.hpp>


class AfterShadow : public DrawObj {
protected:
	std::vector<Model *> cubes;
	glm::mat4 p;
	std::shared_ptr<ShaderMan> shader;
//	const ShaderMan *shader;
	//FBobject depth_frambuffer;
	GLuint depthTex, cubeTex;
public:
//	AfterShadow(ShaderMan *shader);
	AfterShadow(void);
	void setCubeTex(GLuint tex_id);
	int init_setup(void) override;
	int itr_setup(void) override;
	int itr_draw(void) override;
	void append_model(Model *cube);
};

//this should go to a drawing paradgim
class shadowMap : public DrawObj {
protected:
	std::vector<Model *> cubes;
	std::shared_ptr<ShaderMan> shader;
//	const ShaderMan *shader;
	GLuint fbo, depthTex;
	size_t width, height;
public:
//	shadowMap(ShaderMan *shader);
	shadowMap(void);
	
	GLuint getShadowTex() const;
	int init_setup(void) override;
	int itr_setup(void) override;

	int itr_draw(void) override;
	void append_model(Model *cube);
};


#endif

