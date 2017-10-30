#ifndef PROPERTY_HPP
#define PROPERTY_HPP

#include <utility>
#include <tuple>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "types.hpp"

class Model;

struct mesh_GPU_handle {
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	
	mesh_GPU_handle();
	~mesh_GPU_handle();
};

class OBJproperty {
//this class will be the plugins in the model class and mesh class, if the model
//added the property, the mesh should
protected:
	//some of the property has access to GPU, the second is the span
	std::pair<int, int> shader_layouts;
	//the span is determined by the subclasses, you have to write it yourself
	int drawpoint;
	const Model* model;

public:
	OBJproperty()  {
		this->shader_layouts.first = 0;
		this->shader_layouts.second = 0;
		this->drawpoint = false;
	}
	virtual ~OBJproperty() {}
	//return success or not
	virtual bool load(const aiScene *scene) {return true;}
	virtual bool push2GPU(void) {return true;}
	virtual void draw(const msg_t) {}
	void bindModel(const Model *model);
	//some of the property has control to the input of shaders. But not all
	//of them, in that case. It occupies a band of
	void alloc_shader_layout(unsigned int start_point);
	int getLayoutsEnd(void);
	bool isdrawPoint();
};

inline void
OBJproperty::bindModel(const Model *model)
{
	this->model = model;
}

inline void
OBJproperty::alloc_shader_layout(unsigned int start)
{
	this->shader_layouts.first = start;
}

inline int
OBJproperty::getLayoutsEnd(void)
{
	return this->shader_layouts.first + this->shader_layouts.second;
}

inline bool
OBJproperty::isdrawPoint(void)
{
	return this->drawpoint;
}



class Mesh1 : public OBJproperty {
protected:
	int init_options;
	int layout_position, layout_normal, layout_tex;
	
	int nmeshes;
	//GPU handles
	std::vector<mesh_GPU_handle> gpu_handles;
	//CPU copies
	std::vector<Vertices> meshes_vertices;
	std::vector<size_t> material_indices;
	std::vector<Faces> meshes_faces;
public:
	enum OPTION {
		LOAD_NORM = 1,
		LOAD_TEX = 2,
	};
	Mesh1(int option = OPTION::LOAD_NORM | OPTION::LOAD_TEX);
	virtual ~Mesh1() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
	virtual void draw(const msg_t) override;

	uint howmanyMeshes(void) const;
	void activeIthMesh(uint i) const;
	//after loading, this provide a way to exam the meshes
//	std::tuple<Vertices*, size_t *, Faces *> select_mesh(size_t i);
};

class Material1 : public OBJproperty {
	
protected:
	std::vector<Material> Materials;
public:
//	Material()
	virtual ~Material1() override;
	virtual bool load(const aiScene *scene) override;
	virtual void draw(const msg_t) override;
};

class Instancing : public OBJproperty {
protected:
	RSTs rsts;
	GLuint instanceVBO;

public:
	enum OPTION {
		random_instances,
		square_instances,
	};
	
	Instancing(const RSTs& instances);
	Instancing(const int n, const OPTION opt=OPTION::square_instances,
		   const glm::vec3 dscale=glm::vec3(1.0f),
		   const glm::vec3 dtrans=glm::vec3(0.0f),
		   const glm::quat droate=glm::quat(glm::vec3(0.0f)));
	virtual ~Instancing() override;
	//so our instancing doesn't need to have a loading function	
	virtual bool push2GPU(void) override;
	virtual void draw(const msg_t) override;
};


#endif /* EOF */
