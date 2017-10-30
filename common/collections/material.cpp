#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <algorithm>

#include <types.hpp>
#include <property.hpp>
#include <model.hpp>
#include <shaderman.h>

Material1::~Material1()
{
	for (uint i = 0; i < this->Materials.size(); i++)
		for (uint j = 0; j < this->Materials[i].size(); j++)
			glDeleteTextures(1, &(this->Materials[i][j].id));
}

bool
Material1::load(const aiScene *scene)
{
	std::map<std::string, GLuint> textures_cache;
	std::string root_path = this->model->getRootPath();

	this->Materials.resize(scene->mNumMaterials);
	for (GLuint i = 0; i < scene->mNumMaterials; i++) {
		std::cerr << "material indx: " << i << std::endl;
		aiMaterial *mat = scene->mMaterials[i];
		aiString path;
		Material material;
		GLuint gpu_handle;
		for (GLuint j = 0; j < TEX_NTexType; j++) {
			if (mat->GetTextureCount(texture_types_supported[j].aiTextype) > 0) {
				mat->GetTexture(texture_types_supported[j].aiTextype, 0, &path);
				std::string full_path = root_path + "/" + std::string(path.C_Str());
				
				auto it = textures_cache.find(full_path);
				if (it == textures_cache.end()) {
					gpu_handle = load2DTexture2GPU(full_path);
					textures_cache.insert(std::make_pair(full_path, gpu_handle));
				} else
					gpu_handle = it->second;
				material.push_back(
					Texture(gpu_handle,
						texture_types_supported[j].ourTextype));
			}
		}
		this->Materials[i] = material;
	}
	return scene->mNumMaterials;
}

void
Material1::draw(const msg_t material_id)
{
	const ShaderMan *sm = this->model->currentShader();
	//assume I already enabled the program
	const Material& mat = this->Materials[material_id.u];
	for (uint i = 0; i < mat.size(); i++) {
		GLuint texi = sm->getTexUniform(mat[i].type);
		glActiveTexture(GL_TEXTURE0 + texi);
		glBindTexture(GL_TEXTURE_2D, mat[i].id);
	}
}


