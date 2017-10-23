#ifndef PROPERTY_HPP
#define PROPERTY_HPP

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

class Mesh : public OBJproperty {
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
	Mesh(int option = OPTION::LOAD_NORM | OPTION::LOAD_TEX, int layout_start = 0);
	virtual ~Mesh() override;
	virtual bool load(const aiScene *scene) override;
	virtual bool push2GPU(void) override;
	
	//after loading, this provide a way to exam the meshes
	std::tuple<Vertices*, size_t *, Faces *> select_mesh(size_t i);
};



#endif /* EOF */
