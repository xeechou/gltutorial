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
#include "property.hpp"
#include "context.hpp"


class Mesh;
class Texture;
class Model;


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
	//the index of the bone
	int _index;
	glm::mat4 _invTransform;
public:
	Bone(const std::string id = "", const glm::mat4& m = glm::mat4(1.0f));
	Bone(const Bone& bone);
	int getInd() const {return _index;}
	void setInd(int ind) {_index = ind;}
	void setStackedTransformMat() override;
};

//gota decide whether to 
class JointTransform {
public:
	glm::vec3 translation;
	glm::quat rotation;
	glm::vec3 scale;
	std::string joint_name;

	JointTransform(const std::string& name = "",
		const glm::vec3& t = glm::vec3(0.0f),
		const glm::quat& r = glm::quat(glm::vec3(0.0f)),
		const glm::vec3& s = glm::vec3(1.0f));
	const glm::mat4 getLocalTransform();
	static JointTransform interpolate(const JointTransform& a, const JointTransform& b, float progression);
	static glm::vec3 interpolate(const glm::vec3& a, const glm::vec3& b, float progression);
};


struct KeyFrame {
	//for the animation's convience, we need the datatype cooperates with time
	float timeStamp;
	std::map<std::string, JointTransform> transforms;
//	std::vector<JointTransform> transforms;
};

struct Animation {
	double seconds;
	//you r gonna have memory problem with it.
	std::vector<KeyFrame> keyframes;
};

class Model {
	friend Mesh;
protected:
	/**
	 * 
	 * @brief read the model with importer, then disconnect the scene from
	 * the importer, this is however, very important
	 */
	aiScene* readModel(const std::string& filename);
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
	 * @brief use naive BFS method to find the root node
	 */
	const Bone *findRootBone(const aiScene *scene, const aiNode *node);	
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
	
//	std::pair<>
	/**
	 * @brief load animation if exists return how many animations available
	 */
	int loadAnimations(const aiScene *scene);
	
	/**
	 * @brief load materials
	 */
	int loadMaterials(const aiScene *scene);
		
	//
	// Data
	//
	std::string root_path;
	const ShaderMan *shader_to_draw;

	std::vector< std::pair<std::string, std::shared_ptr<OBJproperty> > > properties;
//	std::map< std::string, std::shared_ptr<OBJproperty> > properties;
	
	//TODO: implement mesh, animation, material as properties, and remove
	//the one below
	Instances instances;
	const Bone *root_bone;	
	std::map<std::string, Bone> bones;
	std::vector<Mesh> meshes;
	std::vector<Material> Materials;

	//In case we have different animations
	std::map<std::string, struct Animation> animations;
	//GL interfaces
	GLuint instanceVBO = 0;
	int n_mesh_layouts;
	//TODO: remove the ones above
	
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
		LOAD_ANIM   = 8,
       };
        /**
	 * @brief model constructor, loading meshes and textures, 
	 * 
	 */
	Model(const std::string& file, int params = NO_PARAMS);
	Model(void);
	~Model(void);

	void load(const std::string& file);
	void push2GPU(void);
	void addProperty(const std::string& name, std::shared_ptr<OBJproperty> data);
	
	//you should actually draw with the shaderMan
	void draw(const ShaderMan *differentShader=NULL);
	void setShader(const ShaderMan*);
	//bind, unbind shader
	void bindShader(const ShaderMan *sm) {this->shader_to_draw = sm;}
	const ShaderMan* currentShader(void) {return this->shader_to_draw;}
	

	//get methods
	//TODO: remove this layout
	int getLayoutCount() const {return this->n_mesh_layouts;}
	int getNinstances() const {return this->instances.translations.size(); }

	//and those too, we could also implement instance as properties, as they are no special at all.
	void pushIntances2GPU(void);
	void push2GPU(int param);
  
	//instancing interfaces
	void appendInstance(const glm::vec3 translation,
			    const glm::vec3 scale=glm::vec3(1.0f),
			    const glm::quat rotation=glm::quat(glm::vec3(0.0f)));
	//make many instance
	void makeInstances(const int n_instances, const InstanceINIT method =INIT_squares,
			    //additional arguments maybe useful for different case, or ignored
			    const glm::vec3 dscale=glm::vec3(1.0f),
			    const glm::vec3 dtrans=glm::vec3(0.0f),
			    const glm::quat drotat=glm::quat(glm::vec3(0.0f)));
};


class drawModel : public  DrawObj {
	
};

//we should have this as default. Since if the only on shader is attached. The
//Behavior is supposed to the same as drawModel
class multiPassDrawModel : public MultiPassDrawObj {
public:
	void addModel(std::shared_ptr<Model>& model, const std::string="");
	
};
//now, define a bunch of functions
GLint loadTexture2GPU(const std::string fname);

#endif /* EOF */
