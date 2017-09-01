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

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "types.hpp"
#include "tree.hpp"
#include "shaderman.h"


class Mesh;
class Texture;
class Model;

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

typedef std::vector<Texture> Material;

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

	//use draw_triangles instead of draw_elements. if no_indices is specified. Efficient for small objects
	Mesh(const float *vertx, const float *norms, const float *uvs, const int nnodes,
	     const float *indices = NULL, const int nfaces = 0);
	~Mesh();	
	
	//GPU representation
	GLuint VAO;
	GLuint VBO, EBO;
	//CPU representation
	struct Vertices vertices;
	std::vector<GLuint> indices;

	int materialIndx;
	/**
	 * @brief Loading data to GPU
	 * 
	 * @param params, which data is avaiable for GPU
	 */
	void pushMesh2GPU(int params = LOAD_POS | LOAD_NORMAL | LOAD_TEX);
	/**
	 * @brief Draw current mesh data
	 * 
	 * @param sm The shader to draw
	 * @param model The model that contains the materials
	 */
	void draw(const ShaderMan *sm, const Model& model);
	/**
	 * @brief Loading the bone weights for current mesh
	 * @param mesh The assimp mesh data that contains the bone info
	 * @param model The model data data that we extract the bone list from
	 */
	void loadBoneWeights(aiMesh *mesh, const Model& model);
	
	//optional functions
	//Bone information about the mesh
	Eigen::SparseMatrix<float> boneWeights;
	//matrix that transfer vertices from mesh space to bone space
	glm::mat4 offsetMat;
	//and the tree node
};

class Bone : public TreeNode {
	///matrix that transfer the vertices from mesh(world) space to bone space.
	glm::mat4 _offsetMat;
	int _index;
public:
	Bone(const std::string id = "", const glm::mat4& m = glm::mat4(1.0f));
	Bone(const Bone& bone);
	int getInd() const {return _index;}
	void setInd(int ind) {_index = ind;}
};


class Model {
//it is too late for me to switch to template	
	friend Mesh;
protected:
	/**
	 *
	 * @brief find the heirachy
	 * 
	 * @param scene AiScene to search info
	 * @param node Node to start with
	 *
	 * The method is recursively through a set of bone node. The root node is
	 * usually not a bone, so it will fail if you call this on root
	 * node. The CORRECT method for a scene is more complicated, but here we
	 * are only to load one bone, we just need to find the first bone to
	 * start with.
	 */
	Bone *processBoneNode(const aiScene *scene, const aiNode *node);
	/**
	 *
	 * @brief load every bone into bones
	 * 
	 * It must iterate through all nodes so we wont miss any bone.
	 */
	int loadBone(const aiScene *scene, const aiNode *node);
	/**
	 *
	 * @brief find the first bone
	 *
	 * Use BFS search to find the first bone
	 */
	const Bone *findRootBone(const aiScene *scene, const aiNode *node);
	//
	// Data
	//
	//bones
	const Bone *root_bone;
	std::map<std::string, Bone> bones;
	
	std::vector<Mesh> meshes;
	//materials is a vector of vector
	std::vector<Material> Materials;
	
	std::string root_path;
	const ShaderMan *shader_to_draw;
	//std::vector<Model *> children;
	//I should probaly copy the bone to the instance as well
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
		LOAD_BONE   = 4,
       };
	//Model *modelFromFile(const std::string& file), we could loaded instance nodes from here
	Model(const std::string& file, int params = NO_PARAMS);
	Model(void);
	~Model(void);
	//you should actually draw with the shaderMan
	void draw(const ShaderMan *differentShader=NULL);
	void setShader(const ShaderMan*);
	//bind, unbind shader
	void bindShader(const ShaderMan *sm) {this->shader_to_draw = sm;}
	const ShaderMan* currentShader(void) {return this->shader_to_draw;}
	//get methods
	int getLayoutCount() const {return this->n_mesh_layouts;}
	int getNinstances() const {return this->instances.translations.size(); }
	
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
	void appendInstance(const glm::vec3 translation,
			     const glm::vec3 scale=glm::vec3(1.0f),
			     const glm::quat rotation=glm::quat(glm::vec3(0.0f))) {
		this->instances.translations.push_back(translation);
		this->instances.scales.push_back(scale);
		this->instances.rotations.push_back(rotation);
	}
	//also call the instances2GPU 
	void makeInstances(const int n_instances, const InstanceINIT method =INIT_squares,
			    //additional arguments maybe useful for different case, or ignored
			    const glm::vec3 dscale=glm::vec3(1.0f),
			    const glm::vec3 dtrans=glm::vec3(0.0f),
			    const glm::quat drotat=glm::quat(glm::vec3(0.0f)));
};




//now, define a bunch of functions
GLint loadTexture2GPU(const std::string fname);

#endif /* EOF */
