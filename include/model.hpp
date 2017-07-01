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


class Mesh;
class Texture;
class Model;
class Instances;
	
struct Vertex {
	glm::vec3 Position; //sizeof(glm::vec3) == 12
	glm::vec3 Normal; 
	glm::vec2 TexCoords; //sizeof(glm::vec2) == 8
};

typedef struct Vertex Vertex;

/** UV mapping **/
class Texture {
public:
	//GPU representation
	GLuint id;
	enum TYPE {
		Diffuse,
		Specular,
		Ambient,
		Normal,
		NTypeTexture = 4
	};
	TYPE type;
//		cv::Mat texture;
	Texture(GLuint gpu_handle, TYPE type) {
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
public:
	//a mesh contains the material
	//this may not be a good constructor
	Mesh(const aiScene *scene, aiMesh *mesh);
	//the constructor for non-texture mesh
	Mesh(const std::vector<glm::vec3>& vertxs,
	     const std::vector<glm::vec3>& norms,
	     const std::vector<float>& indices,
	     const std::vector<glm::vec2> *uvs = NULL,
	     const unsigned int material_id = -1);
	~Mesh();
private:
	//GPU representation
	GLuint VAO;
	GLuint VBO, EBO;
	//CPU representation
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	//since the texture is stored with scene, not 
	int materialIndx;
	//push vertices to gpu
	void pushMesh2GPU();
	void draw(GLuint prog, const Model& model);
};


//A model is a list of Meshes
class Model {
	friend Mesh;
private:
	//you can't actually draw one VBO at a time.
	GLuint VAO;
	//Each mesh coordinates is in the model coordinate system, this is how it works
	std::vector<Mesh> meshes;
	//materials is a vector of vector
	std::vector<Material> Materials;
	std::string root_path;
	int processNode(const aiScene *scene, aiNode *node);
		
public:
	//Model *modelFromFile(const std::string& file);
	Model(const std::string& file);
	Model(void);
	~Model(void);
	void draw(GLuint shader);
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
