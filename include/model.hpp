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
	friend Model;
	enum PARAMS {LOAD_POS=1, LOAD_NORMAL=2, LOAD_TEX=4};
private:
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
public:
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
	//add a callback to user. 
};

//A model is a list of Meshes
class Model {
	friend Mesh;
protected:
	enum Parameter {
		NO_PARAMS   = 0,
		NO_TEXTURE  = 1,
		AUTO_NORMAL = 2,
	};
	//you can't actually draw one VBO at a time.
	GLuint VAO;
	//Each mesh coordinates is in the model coordinate system, this is how it works
	std::vector<Mesh> meshes;
	//materials is a vector of vector
	std::vector<Material> Materials;
	std::string root_path;
	const ShaderMan *shader_to_draw;	
	int processNode(const aiScene *scene, aiNode *node);

public:
	//Model *modelFromFile(const std::string& file);
	Model(const std::string& file, Parameter params = NO_PARAMS);
	Model(void);
	~Model(void);
	//you should actually draw with the shaderMan
	void draw(void);
	void setShader(const ShaderMan*);
	//bind, unbind shader
	void bindShader(const ShaderMan *sm) {this->shader_to_draw = sm;}
	const ShaderMan* currentShader(void) {return this->shader_to_draw;}
};


/* some special models to create */
class CubeModel : Model {
	
public:
	//this will give you a one-by-one cube
	CubeModel(void);
	CubeModel(const glm::vec3 translation, const glm::vec3 scale, const glm::vec3 rotation);
	//void SetColor(glm::vec4 color);
};

//third layer of the geometry
class Instances {
private:
	//this is like the animations
	std::vector<glm::vec3> positions;
	std::vector<glm::mat3> rotations;
//		std::vector<glm::vec3> traslations;
	//we don't control the model here
	const Model& target;

public:
	enum INIT {
		INIT_random, //randomly initialize n 
		INIT_squares,
	};
	//initialize n instances based on the flags.
	Instances(const Model& target, const int n_instances = 1, INIT flag = INIT_random);
	void appendInstance(const glm::vec3& positon, const glm::mat3& rotation);
	//later on you need to figure out how to pass mats to GPU, or you could
	//uses the draw instance function
	void instaceMats(std::vector<glm::mat4>& mats);
};


//now, define a bunch of functions
GLuint loadTexture2GPU(const std::string fname);




#endif /* EOF */
