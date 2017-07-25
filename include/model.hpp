#ifndef T_MODEL_HPP
#define T_MODEL_HPP

#include <vector>
#include <string>
#include <algorithm>
#include <map>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "types.hpp"
#include "shaderman.h"

class Mesh;
class Texture;
class Model;
class Instances;

struct Instances {
	std::vector<glm::vec3> translations;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 tex;
};

struct Vertices {
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec2> TexCoords;
};

/** UV mapping **/
class Texture {
public:
	//GPU representation
	GLuint id;
	TEX_TYPE type;

	Texture(GLuint gpu_handle, TEX_TYPE type) {
		this->id = gpu_handle;
		this->type = type;
	}
};

typedef struct Texture Texture;
typedef std::vector<Texture> Material;
//there are more texture types you know
//	typedef std::array<Texture, Texture::NTypeTexture> Material;

class Mesh {
public:
	enum PARAMS {LOAD_POS=1, LOAD_NORMAL=2, LOAD_TEX=4};
	//a mesh contains the material
	//this may not be a good constructor
	Mesh(const aiScene *scene, aiMesh *mesh);
	//the constructor for non-texture mesh
	Mesh(const std::vector<glm::vec3>& vertxs,
	     const std::vector<glm::vec3>& norms,
	     const std::vector<float>& indices,
	     const std::vector<glm::vec2>& uvs = std::vector<glm::vec2>(),
	     const unsigned int material_id = -1);
	~Mesh();
	
	//use draw_triangles instead of draw_elements. if no_indices is specified. Efficient for small objects
	Mesh(const float *vertx, const float *norms, const float *uvs, const int nnodes,
	     const float *indices = NULL, const int nfaces = 0);
	
	//Okay, Mesh is naked now.
	//GPU representation
	GLuint VAO;
	GLuint VBO, EBO;
	//CPU representation
	struct Vertices vertices;
	std::vector<GLuint> indices;
	//since the texture is stored with scene, not 
	int materialIndx;
	//push vertices to gpu
	void pushMesh2GPU(int params = LOAD_POS | LOAD_NORMAL | LOAD_TEX);
	void draw(GLuint prog, const Model& model);
	void draw(const ShaderMan *sm, const Model& model);
	//add a callback to user. 
};

//It should be a tree later on
class Model {
	friend Mesh;
	friend Instances;
protected:
	int processNode(const aiScene *scene, aiNode *node);	

	//Each mesh coordinates is in the model coordinate system, this is how it works
	std::vector<Mesh> meshes;
	//materials is a vector of vector
	std::vector<Material> Materials;
	std::string root_path;
	const ShaderMan *shader_to_draw;
	//std::vector<Model *> children;
	struct Instances instances;
	//GL interfaces
	GLuint instanceVBO = 0;
	int n_mesh_layouts;

public:
	enum InstanceINIT {
		INIT_random, //randomly initialize n 
		INIT_squares, // n by n from (0,0)
		//TODO
		//INIT_ORBIT //orbit around a position.
	};
	enum Parameter {
		NO_PARAMS   = 0,
		NO_TEXTURE  = 1,
		AUTO_NORMAL = 2,
	};
	
	
	//Model *modelFromFile(const std::string& file), we could loaded instance nodes from here
	Model(const std::string& file, Parameter params = NO_PARAMS);
	Model(void);
	~Model(void);
	//you should actually draw with the shaderMan
	void draw();
	void draw(const ShaderMan *differentShader);
	void setShader(const ShaderMan*);
	//bind, unbind shader
	void bindShader(const ShaderMan *sm) {this->shader_to_draw = sm;}
	const ShaderMan* currentShader(void) {return this->shader_to_draw;}
	//get methods
	int get_layout_count() const {return this->n_mesh_layouts;}
	int get_ninstances() const {return this->instances.translations.size(); }
	
	void pushIntances2GPU(void);
	void push2GPU(int param) {
		//get the proper texture 
		this->n_mesh_layouts = 1;
		if (param & Mesh::LOAD_NORMAL)
			this->n_mesh_layouts += 1;
		if (param & Mesh::LOAD_TEX)
			this->n_mesh_layouts += 1;
		
		for (unsigned int i = 0; i < this->meshes.size(); i++)
			this->meshes[i].pushMesh2GPU(param);
		
		//We can do it here or 
		if (this->instances.translations.size() > 0 && instanceVBO == 0)
			this->pushIntances2GPU();
		
	}
	//instancing interfaces
	void append_instance(const glm::vec3 translation,
			     const glm::vec3 scale=glm::vec3(1.0f),
			     const glm::quat rotation=glm::quat(glm::vec3(0.0f))) {
		this->instances.translations.push_back(translation);
		this->instances.scales.push_back(scale);
		this->instances.rotations.push_back(rotation);
	}
	//also call the instances2GPU 
	void make_instances(const int n_instances, const InstanceINIT method =INIT_squares,
			    //additional arguments maybe useful for different case, or ignored
			    const glm::vec3 dscale=glm::vec3(1.0f),
			    const glm::vec3 dtrans=glm::vec3(0.0f),
			    const glm::quat drotat=glm::quat(glm::vec3(0.0f)));
};


/* this is actually a bad design. I can either write public functions:
   makeCube, makeUVSphere, makeCylinder, makePyramid something like that */
class CubeModel : public Model {
	
public:
	//this will give you a one-by-one cube
	CubeModel(const glm::vec3 translation = glm::vec3(0.0f),
		  const glm::vec3 scale = glm::vec3(1.0f),
		  const glm::quat rotation = glm::quat(glm::vec3(0.0f)));
	//void SetColor(glm::vec4 color);
};


//now, define a bunch of functions
GLuint loadTexture2GPU(const std::string fname);

#endif /* EOF */
