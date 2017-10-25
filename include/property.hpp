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
public:
	OBJproperty() {
		this->shader_layouts.first = 0;
		this->shader_layouts.second = 0;
	}
	virtual ~OBJproperty();
	//return success or not
	virtual bool load(const aiScene *scene) {return true;}
	virtual bool push2GPU(void) {return true;}
	//some of the property has control to the input of shaders. But not all
	//of them, in that case. It occupies a band of
	void alloc_shader_layout(unsigned int start_point);
	int getLayoutsEnd(void);
};

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
		LOAD_NORM = 0,
		LOAD_TEX = 1,
	};
	Mesh1(int option = OPTION::LOAD_NORM | OPTION::LOAD_TEX);
	virtual ~Mesh1() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
	
	//after loading, this provide a way to exam the meshes
	std::tuple<Vertices*, size_t *, Faces *> select_mesh(size_t i);
};



#endif /* EOF */
