#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>
#include <set>
#include <utility>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//define uint now
typedef unsigned int uint;

//well, we end up make those difficult type just to fix small bugs. Well, how
//can I say. This is definitly not complete. Very first example, we can't get a
//cubemap texture type here, but it is supported very well by OpenGL.
enum TEX_TYPE {
	TEX_Diffuse=0,
	TEX_Specular=1,
	TEX_Normal=2,
	TEX_Ambient=3,
	TEX_NASSIMP_TYPE=4, //it doesnt support cubemap, we need write a specific loadCubeMap function
	TEX_NTexType=5,	
	TEX_CubeMap=6,

};

typedef struct {
	aiTextureType aiTextype;
	TEX_TYPE ourTextype;
} sprt_tex2d_t;


extern sprt_tex2d_t texture_types_supported[TEX_NASSIMP_TYPE];

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



//here is how you draw the texture if it is supported by assimp
//for
//for (int i = 0; i < shader->getSupported_texture_uniforms; i++)
//  glActiveTexture(GL_TEXTURE0 + shader->support_texture_start_indx + i)
//  glBindTexture(GL_TEXTURE_2D, matid_someting)

//This msg_t has to be POD data type.
//the message system is used by draw objects to aquire input and output data
typedef union msg_t {
	uint32_t u;
	int32_t  i;
	float f;
	double d;
	const char *name;
	bool b;
	msg_t() {this->d=0.0;}
	msg_t(uint32_t ud) {this->u = ud;}
	msg_t(int32_t  id) {this->i = id;}
	msg_t(float    fd) {this->f = fd;}
	msg_t(double   dd) {this->d = dd;}
	msg_t(const char *n) {this->name = n;}
	msg_t(bool _b) {this->b = _b; }
} msg_t;

//Other more specific types
typedef std::set<double> timestamps_t;

struct RSTs {
	std::vector<glm::vec3> translations;
	std::vector<glm::quat> rotations;
	std::vector<glm::vec3> scales;

	void addInstance(const glm::vec3 p,
			 const glm::quat r=glm::quat(glm::vec3(0.0)),
			 const glm::vec3 s=glm::vec3(1.0f));
};

inline void
RSTs::addInstance(const glm::vec3 p, const glm::quat r, const glm::vec3 s)
{
	this->translations.push_back(p);
	this->rotations.push_back(r);
	this->scales.push_back(s);
}


typedef struct RSTs Instances;

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


typedef glm::u32vec3 face_t;
typedef std::vector<face_t> Faces;

#endif /* EOF */
