#ifndef T_MESH_HPP
#define T_MESH_HPP

#include <iostream>
#include <sstream>
#include <vector>
#include <map>

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

/** UV mapping **/
struct Texture {
	GLuint id;
	std::string type; ///diffuse or specular
	std::string path; ///The relative path to the texture
};

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



class Model {
public:
	Model(GLchar *path) {this->loadModel(path);}
	void Draw(GLuint shader);
private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::map<std::string, Texture> textures_loaded;

	//private methods
	void loadModel(const char *path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(const aiMaterial* mat, aiTextureType type, const char *name);
};


#endif
