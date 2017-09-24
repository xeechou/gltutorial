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
