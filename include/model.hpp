#ifndef T_MESH_HPP
#define T_MESH_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <map>
#include <tuple>
#include <string>


#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>


//asset loading library
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


//The Vertex struct is one row of vertex. We need to makesure there is no hole between data
struct Vertex {
	glm::vec3 Position; //sizeof(glm::vec3) == 12
	glm::vec3 Normal; 
	glm::vec2 TexCoords; //sizeof(glm::vec2) == 8
};

typedef struct Vertex Vertex;

/** UV mapping **/
struct Texture {
	GLuint id;
	std::string type; ///diffuse or specular
	std::string path; ///The relative path to the texture
};
typedef struct Texture Texture;

class Mesh {
public:
	/* Mesh data */
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	void Draw(GLuint);
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
private:
	GLuint VAO, VBO, EBO;
	void setupMesh();
};


/**
 * @brief The basic model class, only contains a list of meshes
 */
class Model {
public:
	Model(GLchar *path) {this->loadModel(path);}
	void Draw(GLuint shader);
private:
	//this is actually bad, after you loading all the meshes, the heirarchy
	//info lost, we need to change the code
	std::vector<Mesh> meshes;
	std::string directory;
	std::map<std::string, Texture> textures_loaded;

	//private methods
	void loadModel(const char *path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(const aiMaterial* mat, aiTextureType type, const char *name);
};

enum TYPE_LIGHT {
	POINT,
	DIRECTION,
	SPOT,
};

struct light {
	glm::vec3 color;
	float ambient_intensity;
	float diffuse_intensity;
	//and there more stuff.
};






////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////New Stuff////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


//This code is pretty much simulating the assimp data structure just for loading data to OpenGL, we should naming it after GL
class MeshNode;
class ModelNode;
class Scene;

//the size_t here is the node_id, so we know what does the transformation for,
//by the time processing mesh, we should already have the node_id in each transform
typedef std::pair<std::string, ModelNode> modelnode_t;

enum Material_type {
	TEX_AMBIENT_MAP,
	TEX_DIFFUSE_MAP,
	TEX_SPECULAR_MAP,
	TEX_NORMAL_MAP,
	TEX_UNKNOWN_MAP
};
#define NUM_MATERIAL_TYPE 4

struct Tex {
	GLint64 id;
	Material_type type;
};


///
/// The mesh also contains a transformation, different material access as well, this is super generic
class MeshNode {
	friend class Scene;
	friend class ModelNode;
private:
	unsigned int model_id;
	GLuint VAO, VBO, EBO, instanceTBO, instanceTex;
	std::vector<struct Vertex> vertices;
	std::vector<GLuint> indices;
	//we should have it access to material
	std::vector< std::pair<int, aiMatrix4x4> > transforms;

	//The same mesh, but we want to use different materials, this could be a piece of work

	//based on assimp, every mesh only use one material
//	std::vector<GLuint > materials;
	//this maynot be a good idea, but assimp only support one material at a time
	std::vector<unsigned int>materials;
	virtual void setupMesh_forshader(GLuint program, Scene* scene);

public:
	MeshNode() {VAO = VBO = EBO = 0;}
	~MeshNode() {
		if (EBO)
			glDeleteBuffers(1, &EBO);
		if (VBO)
			glDeleteBuffers(1, &VBO);
		if (instanceTBO)
			glDeleteBuffers(1, &instanceTBO);
		if (instanceTex)
			glDeleteTextures(1, &instanceTex);
		if (VAO)
			glDeleteVertexArrays(1, &VAO);
	}
	//all the method involve OpenGL code should be declared as virtual
	virtual void draw(GLuint program, Scene* scene);
	virtual void drawInstanced(GLuint program, Scene* scene);
};


//A node can have one or more meshes. 
class ModelNode {
	friend class MeshNode;
	friend class Scene;
private:
	int node_id, parent;

	//the handle for meshes;
	std::vector<unsigned int> meshes;
	std::vector<unsigned int> child_nodes; //Refer by id so we can avoid cross reference problem
	glm::mat4 ModelMat;

	std::string directory;   //this is useful for use to load textures.
	//Maybe 
public:
	ModelNode(int id) {this->node_id = id;}
	ModelNode(int id, int pid) {this->node_id = id; this->parent = pid;}
	void setParent(unsigned int parent_id);
	void processNode(aiNode *ainode, const aiScene *aiscene, Scene *scene,
			 //data terms
			 unsigned int model_id,
			 const std::string& directory, const aiMatrix4x4& transform);
};

typedef std::tuple<std::vector<unsigned int>, unsigned int> model_ind_t;
/**
 * @brief The basic Scene data structure
 *
 *
 * For now it is super simple, We can just use model for the rendering.
 * This time model has to be something different. Multiple models need to share the same meshes,
 * and the I should be able to load the scene with mixed mesh strategy and assimp does, I should also code a shader-interface with it.
 * TODO: And we also need a shader interface to deal with all the bullshit.
 */
class Scene {
	friend class ModelNode;
	friend class MeshNode;
	
private:

	//data
	std::vector<struct light> light_sources;
	std::vector<std::pair<std::string, ModelNode> > nodes;	///< all the child nodes info is here
	std::vector<MeshNode> meshes;

	//materials is a group of textures, we would have many diffuse/specular map
	std::vector<std::pair<std::string, struct Tex> > materials;


	//! First level node is different than others
	std::vector<std::tuple<std::vector<unsigned int>, //mesh index for the model
			       unsigned int> //The model id
		    > children;

	std::map<unsigned int, GLuint > instances; //this is for instancing

	//the member name here is terrible, `loaded` means loaded stuff before this processingNode
	size_t loaded_mesh, loaded_nodes, loaded_material;

	//TODO: animation files and bone files

	//utils: check functions
	MeshNode *getMesh(size_t mid) {
		return ((mid + loaded_mesh) < this->meshes.size()) ? &this->meshes[mid+loaded_mesh]: NULL;
	}
protected:
	unsigned int getMaterialID(unsigned int aiMatID, enum Material_type type) {
		return this->loaded_material + aiMatID * NUM_MATERIAL_TYPE + type;
	}
	
	void append_mesh(aiMesh* mesh, const aiScene *scene, unsigned int model_id);
	virtual void processingMaterial(const aiMaterial *, const std::string&);
	virtual GLuint setupModelInstance(const std::vector<glm::mat4>& );
public:
	/// \brief iterate through meshes and render it.
	Scene(void) {
		nodes.clear(); children.clear(); meshes.clear();
		loaded_nodes = 0; loaded_material = 0; loaded_mesh = 0;
	}
	//after you done with loading model, you should setup the

	void loadModel(const char *path, const std::vector<glm::mat4>& instances_transform);	
	void loadModel(const char *path) {this->loadModel(path, std::vector<glm::mat4>(0));}


	void draw(GLuint program);
};


#endif /* EOF */
