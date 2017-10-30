
#include <iostream>
#include <tuple>
#include <types.hpp>
#include <model.hpp>
#include <property.hpp>



mesh_GPU_handle::mesh_GPU_handle()
{
	this->VAO = 0;
	this->VBO = 0;
	this->EBO = 0;
}

mesh_GPU_handle::~mesh_GPU_handle()
{
	if (this->VAO) {
		glDeleteVertexArrays(1, &this->VAO);
		this->VAO = 0;
	}
	if (this->VBO) {
		glDeleteBuffers(1, &this->VBO);
		this->VBO = 0;
	}
	if (this->EBO) {
		glDeleteBuffers(1, &this->EBO);
		this->EBO = 0;
	}
}

Mesh1::Mesh1(int option)
{
	this->drawpoint=true;
	int layout_span = 1;
	this->init_options = option;
	if (option & OPTION::LOAD_NORM) {
		layout_span += 1;
	} if (option & OPTION::LOAD_TEX) {
		layout_span += 1;
	}
	this->shader_layouts.second=layout_span;
}


Mesh1::~Mesh1()
{}

bool
Mesh1::load(const aiScene *scene)
{
	int has_tex = 0;
	int has_norm = 0;
	//allocate memory
	this->meshes_vertices.resize(scene->mNumMeshes);
	this->material_indices.resize(scene->mNumMeshes);
	this->meshes_faces.resize(scene->mNumMeshes);
	
	for (size_t j = 0; j < scene->mNumMeshes; j++) {
		aiMesh *mesh = scene->mMeshes[j];
		this->material_indices[j] = (int)mesh->mMaterialIndex;
		//geometry data
		std::vector<glm::vec3> &poses = this->meshes_vertices[j].Positions;
		std::vector<glm::vec3> &norms = this->meshes_vertices[j].Normals;
		std::vector<glm::vec2> &texuvs= this->meshes_vertices[j].TexCoords;
		Faces  &faces =  this->meshes_faces[j];
		
		poses.resize(mesh->mNumVertices);
		if (this->init_options & OPTION::LOAD_NORM) {
			norms.resize(mesh->mNumVertices);
			has_norm = 1;
		}
		//agian, we only have one tex coordinate system
		if (mesh->mTextureCoords[0] && (this->init_options & OPTION::LOAD_TEX)) {
			texuvs.resize(mesh->mNumVertices);
			has_tex = 1;
		}
	
		for (GLuint i = 0; i < mesh->mNumVertices; i++) {
			poses[i] = glm::vec3(mesh->mVertices[i].x,
					     mesh->mVertices[i].y,
					     mesh->mVertices[i].z);
			if (has_norm)
				norms[i] = glm::vec3(mesh->mNormals[i].x,
						     mesh->mNormals[i].y,
						     mesh->mNormals[i].z);
			if (has_tex)
				texuvs[i] = glm::vec2(mesh->mTextureCoords[0][i].x,
						      mesh->mTextureCoords[0][i].y);
		}
		faces.resize(mesh->mNumFaces);
		for (GLuint i = 0; i < mesh->mNumFaces; i++) {
			//one face should only have 3 indices
			aiFace face = mesh->mFaces[i];
			faces[i][0] = face.mIndices[0];
			faces[i][1] = face.mIndices[1];
			faces[i][2] = face.mIndices[2];
		}
	}
	return true;
}

bool
Mesh1::push2GPU()
{
	//okay, figure out the layout
	this->layout_position = this->shader_layouts.first;
	if (this->shader_layouts.second > 1)
		this->layout_normal = this->shader_layouts.first + 1;
	if (this->shader_layouts.second > 2)
		this->layout_tex = this->shader_layouts.first + 2;

	
	this->gpu_handles.resize(this->meshes_vertices.size());
	int stride = sizeof(glm::vec3) +
		((this->init_options & OPTION::LOAD_NORM) ? sizeof(glm::vec3) : 0) +
		((this->init_options & OPTION::LOAD_TEX) ? sizeof(glm::vec2) : 0);
	
	for (size_t j = 0; j < this->gpu_handles.size(); j++) {
		mesh_GPU_handle &handle = this->gpu_handles[j];
		std::vector<glm::vec3> &poses = this->meshes_vertices[j].Positions;
		std::vector<glm::vec3> &norms = this->meshes_vertices[j].Normals;
		std::vector<glm::vec2> &texuvs= this->meshes_vertices[j].TexCoords;
		Faces  &faces =  this->meshes_faces[j];
		
		glGenVertexArrays(1, &handle.VAO);
		glGenBuffers(1, &handle.VBO);
		glGenBuffers(1, &handle.EBO);

		glBindVertexArray(handle.VAO);
		//EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(glm::u32vec3),
			     &faces[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		//VBO
		glBindBuffer(GL_ARRAY_BUFFER, handle.VBO);
		glBufferData(GL_ARRAY_BUFFER, poses.size() * stride,
			     NULL, GL_STATIC_DRAW);
		
		size_t offset = 0;
		glBufferSubData(GL_ARRAY_BUFFER, offset, poses.size() * sizeof(glm::vec3), &poses[0]);
		glEnableVertexAttribArray(this->layout_position);
		glVertexAttribPointer(this->layout_position, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid *)offset);
		offset += poses.size() * sizeof(glm::vec3);
		
		if (this->init_options & OPTION::LOAD_NORM) {
			glBufferSubData(GL_ARRAY_BUFFER, offset, norms.size() * sizeof(glm::vec3), &norms[0]);
			//okay, I am confused, do I need to call this like all the time?
			glEnableVertexAttribArray(this->layout_normal);
			glVertexAttribPointer(this->layout_normal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid *)offset);
			offset += norms.size() * sizeof(glm::vec3);
		}
		if (this->init_options & OPTION::LOAD_TEX) {
			glBufferSubData(GL_ARRAY_BUFFER, offset, texuvs.size() * sizeof(glm::vec2), &texuvs[0]);
			glEnableVertexAttribArray(this->layout_tex);
			glVertexAttribPointer(this->layout_tex, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid *)offset);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//de-assign
		glBindVertexArray(0);

	}
	return true;
}

void
Mesh1::draw(const msg_t arg)
{
	(void)arg;
	Material1 *material = (Material1*)this->model->searchProperty(std::string("material"));

	for (uint i = 0; i < this->gpu_handles.size(); i++) {
		if (material)
			material->draw( msg_t((uint32_t)this->material_indices[i]) );
		
		mesh_GPU_handle &handle = this->gpu_handles[i];
		Faces& faces = this->meshes_faces[i];
		glBindVertexArray(handle.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, handle.VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.EBO);
		glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
		//other drawing function may also need to work, like animations and instancing.
	}
}

std::tuple<Vertices*, size_t *, Faces *>
Mesh1::select_mesh(size_t i)
{
	
}

