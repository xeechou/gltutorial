#include <collections/shaders.hpp>


shaderShadowMap::shaderShadowMap(void)
{
	const std::string vs_source =
#include <collections/shaders/shadowmapvs.glsl>
		;
	const std::string fs_source =
#include <collections/shaders/shadowmapfs.glsl>
		;
	this->shaders.resize(2);
	shaders[0] = ShaderMan::createShaderFromString(vs_source, GL_VERTEX_SHADER);
	shaders[1] = ShaderMan::createShaderFromString(fs_source, GL_FRAGMENT_SHADER);
	this->pid = ShaderMan::loadShaderProgram(&shaders[0], shaders.size());
}

phongWithShadowMap::phongWithShadowMap(void)
{
	const std::string vs_source =
#include <collections/shaders/phongvs.glsl>
		;
	const std::string fs_source =
#include <collections/shaders/phongfs.glsl>
		;
	this->shaders.resize(2);
	shaders[0] = ShaderMan::createShaderFromString(vs_source, GL_VERTEX_SHADER);
	shaders[1] = ShaderMan::createShaderFromString(fs_source, GL_FRAGMENT_SHADER);
	this->pid = ShaderMan::loadShaderProgram(&shaders[0], shaders.size());
}


const std::string phongNoShadow::uniform_MVP = "MVP";
const std::string phongNoShadow::uniform_lightPos = "lightPos";
const std::string phongNoShadow::uniform_texdiffuse = "diffuse_tex";
const std::string phongNoShadow::uniform_texspecular = "specular_tex";

phongNoShadow::phongNoShadow(void)
{
	const std::string vs_source =
#include <collections/shaders/phong0vs.glsl>
		;
	const std::string fs_source =
#include <collections/shaders/phong0fs.glsl>
		;
	this->shaders.resize(2);
	shaders[0] = ShaderMan::createShaderFromString(vs_source, GL_VERTEX_SHADER);
	shaders[1] = ShaderMan::createShaderFromString(fs_source, GL_FRAGMENT_SHADER);
	this->pid = ShaderMan::loadShaderProgram(&shaders[0], shaders.size());

	this->useProgram();
	GLuint mvp =  glGetUniformLocation(this->pid, this->uniform_MVP.c_str());
	GLuint lightpos = glGetUniformLocation(this->pid, this->uniform_lightPos.c_str());
	this->uniforms.insert(std::make_pair(this->uniform_MVP, mvp));
	this->uniforms.insert(std::make_pair(this->uniform_lightPos, lightpos));
	this->addTextureUniform(phongNoShadow::uniform_texdiffuse, TEX_Diffuse);
	this->addTextureUniform(phongNoShadow::uniform_texspecular, TEX_Specular);
}
