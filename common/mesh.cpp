//that's enough
#include <mesh.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

static GLuint
texture_from_file(const char *path, std::string& directory)
{
	std::string fname(path);
	fname = directory + '/' + fname;
	GLuint tid;

	glGenTextures(1, &tid);
	glBindTexture(GL_TEXTURE_2D, tid);
	cv::Mat img = cv::imread(fname, CV_LOAD_IMAGE_COLOR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Paramaters
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	return tid;
}



Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->setupMesh();
}

void
Mesh::setupMesh(void)
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	//the stride is sizeof(Vertex), a size of a structure, this is new
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex),
		     &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint),
		     &this->indices[0], GL_STATIC_DRAW);

	//Vertex positons for layouts
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			      (GLvoid *)0);
	//Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			      (GLvoid *)offsetof(Vertex, Normal));
	//Vertex Texture Coords?
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			      (GLvoid *)offsetof(Vertex, TexCoords));
	glBindVertexArray(0);
}

//draw everymesh a round, this code should not be a problem
void Mesh::Draw(GLuint prog_id)
{
	GLuint diffuseNr = 0;
	GLuint specularNr = 0;
	for(GLuint i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // Activate proper texture unit before binding
		// Retrieve texture number (the N in diffuse_textureN)
		std::stringstream ss;
		std::string number;
		std::string name = this->textures[i].type;
		if(name == "diffuse")
			ss << diffuseNr++; // Transfer GLuint to stream
		else if(name == "specular")
			ss << specularNr++; // Transfer GLuint to stream
		number = ss.str();

		glUniform1i(glGetUniformLocation(prog_id, ("material." + name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// Draw mesh
	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


void
Model::Draw(GLuint shader)
{
	std::for_each(this->meshes.begin(), this->meshes.end(),
		      [shader](Mesh &m){m.Draw(shader);});
}

void
Model::loadModel(const char *path)
{
	Assimp::Importer import;
	const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	std::string path_str(path);
	this->directory = path_str.substr(0, path_str.find_last_of('/'));
	this->processNode(scene->mRootNode, scene);
}

///Every node represent a mesh from the image.
//amazing, we have tree-structures already!!!
void
Model::processNode(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for(GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(this->processMesh(mesh, scene));
	}
	// Then do the same for each of its children
	for(GLuint i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}
}

Mesh
Model::processMesh(aiMesh *mesh, const aiScene *scene)
{
	//this is the data to fill
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	for (GLuint i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		//Position
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		//Normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		//UV coordinates
		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			//A vertex can contain up to 8 different texture coordinates, thus we only use the first one
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		} else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}
	//Processing indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	//materials is diffuse and specular
	if (mesh->mMaterialIndex >= 0)
	{
		//material means diffuse and specular 
		const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		//you know diffuse/specular is nothing but samplers for textures.

		//0. Diffuse map
		std::vector<Texture> diffusemaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
		textures.insert(textures.end(), diffusemaps.begin(), diffusemaps.end());
		//1. Specular map
		std::vector<Texture> specularmaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
		textures.insert(textures.end(), specularmaps.begin(), specularmaps.end());
	}

	return Mesh(vertices, indices, textures);
}


std::vector<Texture>
Model::loadMaterialTextures(const aiMaterial* mat, aiTextureType type, const char *name)
{
	std::vector<Texture> textures;
	for (size_t i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string str_fixer(str.C_Str());
		std::cout << str_fixer << std::endl;


		auto it = textures_loaded.find(str_fixer);
		if (it != textures_loaded.end())
			textures.push_back(it->second);
		else {
			Texture texture;
			texture.id = texture_from_file(str.C_Str(), this->directory);
			texture.type = std::string(name);
			texture.path = str_fixer;
			textures.push_back(texture);
			textures_loaded[str_fixer] = texture;
		}
	}
	return textures;
}
