#include <iostream>
#include <tuple>
#include <types.hpp>
#include <model.hpp>
#include <property.hpp>

//not sure if it is a good idea defining here
const ShaderMan*
OBJproperty::getBindedShader(void)
{
	return this->model->currentShader();
}

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

void
Mesh1::layout_count(void)
{
	int layout_span = 1;
	if (this->init_options & OPTION::LOAD_NORM) {
		layout_span += 1;
	} if (this->init_options & OPTION::LOAD_TEX) {
		layout_span += 1;
	}
	this->shader_layouts.second=layout_span;

}


Mesh1::Mesh1(int option)
{
	this->init_options = option;
	layout_count();
}


Mesh1::Mesh1(const float *vertx, const float *norms, const float *uvs, const int nnodes,
	     const float *indices, const int nfaces)
{
	this->init_options = OPTION::LOAD_NORM | OPTION::LOAD_TEX;

	const int size_vn = 3;
	const int size_uv = 2;

	this->meshes_vertices.resize(1);
	this->meshes_faces.resize(1);
	std::vector<glm::vec3> &poses   = this->meshes_vertices[0].Positions;
	std::vector<glm::vec3> &normals = this->meshes_vertices[0].Normals;
	std::vector<glm::vec2> &texuvs  = this->meshes_vertices[0].TexCoords;
	Faces &faces = this->meshes_faces[0];

	poses.resize(nnodes);
	normals.resize(nnodes);
	texuvs.resize(nnodes);
	for (int i = 0; i < nnodes; i++) {
		poses[i]   = glm::make_vec3(i*size_vn + vertx);
		normals[i] = glm::make_vec3(i*size_vn + norms);
		texuvs[i]  = glm::make_vec2(i*size_uv + uvs);
	}
	//deal with indices
	uint nnfaces;
	std::vector<float> indx;
	if (indices) {
		nnfaces = nfaces;
		faces.resize(nfaces);
		std::copy(indices, indices + nfaces *3, indx.begin());
	} else {
		nnfaces = nnodes /3;
		faces.resize(nnfaces);
		int n = {0};
		indx.resize(nnodes);
		std::generate(indx.begin(), indx.end(), [&] {return n++;});
	}
	for (uint i = 0; i < nnfaces; i++)
		faces[i] = glm::u32vec3(indx[3*i], indx[3*i+1], indx[3*i+2]);
	layout_count();

}


Mesh1::	Mesh1(const std::vector<glm::vec3>&& vertx,
	      const std::vector<face_t>&& faces,
	      const std::vector<glm::vec3>&& norms,
	      const std::vector<glm::vec2>&& uvs)
{
	this->init_options = 0;
	this->meshes_vertices.resize(1);
	this->meshes_faces.resize(1);
	this->meshes_vertices[0].Positions = std::move(vertx);
	if (!norms.empty()) {
		this->init_options |= OPTION::LOAD_NORM;
		meshes_vertices[0].Normals = std::move(norms);
	}
	if (!uvs.empty()) {
		this->init_options |= OPTION::LOAD_TEX;
		meshes_vertices[0].TexCoords = std::move(uvs);
	}
	if (faces.empty()) {
		int nfaces = this->meshes_vertices[0].Positions.size() / 3;
		std::vector<face_t> faces(nfaces);
		int idx = 0;
		std::generate(faces.begin(), faces.end(), [&]() {
				int cidx = idx;
				idx+=3;
				return face_t(cidx, cidx+1, cidx+2);
			});
		this->meshes_faces[0] = std::move(faces);
	} else
		this->meshes_faces[0] = std::move(faces);
	layout_count();
}


Mesh1::~Mesh1()
{}

bool
Mesh1::load(const aiScene *scene)
{
	//if we already load everything, then we jump
	if (!this->meshes_vertices.empty())
		return true;
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
		} else if (this->init_options & OPTION::LOAD_TEX) {
			//disable it
			this->init_options |= ~OPTION::LOAD_TEX;
			//and we need cancel the span
			this->shader_layouts.second--;
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
	Instancing *instancing = (Instancing *)this->model->searchProperty("instancing");

	for (uint i = 0; i < this->gpu_handles.size(); i++) {
		if (material)
			material->draw( msg_t((uint32_t)this->material_indices[i]) );

		mesh_GPU_handle &handle = this->gpu_handles[i];
		Faces& faces = this->meshes_faces[i];
		glBindVertexArray(handle.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, handle.VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle.EBO);
		if (instancing)
			instancing->draw(msg_t((uint32_t)(faces.size() *3)));
		else
			glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
	}
}


void
Mesh1::activeIthMesh(uint i) const
{
	if (i >= this->gpu_handles.size())
		return;
	glBindVertexArray(this->gpu_handles[i].VAO);
}

uint
Mesh1::howmanyMeshes() const
{
	return this->gpu_handles.size();
}

/*
std::tuple<Vertices*, size_t *, Faces *>
Mesh1::select_mesh(size_t i)
{

}

*/
