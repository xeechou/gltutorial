#ifndef T_MODEL_HPP
#define T_MODEL_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "operations.hpp"
#include "types.hpp"
#include "tree.hpp"
#include "shaderman.h"
#include "property.hpp"
#include "context.hpp"
#include "collections/bone.hpp"


class Model {
protected:
	/**
	 * @brief read the model with importer, then disconnect the scene from
	 * the importer, this is however, very important
	 */
	aiScene* readModel(const std::string& filename);
	//
	// Data
	//
	std::string root_path;
	const ShaderMan *shader_to_draw;
	std::pair<int, std::shared_ptr<OBJproperty> > draw_point;

	std::map<uint, std::pair<std::string, std::shared_ptr<OBJproperty> > > properties;

public:
	/**
	 * @brief model constructor, loading meshes and textures,
	 *
	 */
	Model(void);
	~Model(void);

	const std::string getRootPath(void) const;
	void load(const std::string& file);
	//seperate load and push2GPU as geometry class may have it useful
	void push2GPU(void);
	bool addProperty(const std::string& name, std::shared_ptr<OBJproperty> data=nullptr);
	//	bool addProperty(int proptype, const std::string& name, std::shared_ptr<OBJproperty> data);
	OBJproperty *searchProperty(const std::string name) const;

	//you should actually draw with the shaderMan
	void draw(const ShaderMan *differentShader=NULL);
	void drawProperty(const ShaderMan *differentShader=NULL);
	//bind, unbind shader
	void bindShader(const ShaderMan *sm) {this->shader_to_draw = sm;}
	const ShaderMan* currentShader(void) const {return this->shader_to_draw;}
};

/**
 * @brief describes the order of properties
 */
/*
A draw Obj can have more model to draw. One model can also bind to multiple
shaders. So you don't really know which to which to which. So maybe next time

template<class Derived>
class DrawModel : public DrawObj {
protected:
	std::string model_file;
	std::shared_ptr<Model> model;
public:
	void addModel(std::shared_ptr<Model>& model, const std::string="");
	virtual int init_setup(void) override;
	virtual int itr_setup(void) override;
	virtual int itr_draw(void) override;
	//you want to get
	void model_init(void);
	void model_itrsetup(void);
	void model_itrdraw(void);

};

template<class Derived> int
DrawModel<Derived>::init_setup(void)
{
	if (model_file.size())
		model->load(model_file);
	static_cast<Derived*>(this)->model_init();
}

template<class Derived> int
DrawModel<Derived>::itr_setup()
{
	static_cast<Derived*>(this)->model_itrsetup();
}

template<class Derived> int
DrawModel<Derived>::itr_draw()
{
	model->draw();
}

template<class Derived> void
DrawModel<Derived>::addModel(std::shared_ptr<Model> &m, const std::string mf)
{
	this->model = m;
	this->model_file = mf;
}
*/

//now, define a bunch of functions


#endif /* EOF */
