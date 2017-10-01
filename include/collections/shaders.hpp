#ifndef T_SHADERS_HPP
#define T_SHADERS_HPP

#include "../shaderman.h"

class shaderShadowMap : public ShaderMan {
public:
	shaderShadowMap(void);
};


class phongWithShadowMap : public ShaderMan {
public:
	phongWithShadowMap(void);
};

//the skybox program,
class shaderSkybox : public ShaderMan {
public:
	shaderSkybox(void);
//	~shaderSkybox(void) {}
	//TODO: interface to add skybox texture interface, if you accept the cpu
	//resource, you should do the freeing, otherwise. You don't need to...
	//this also leads to another problem: if an shader class is binded to
	//drawOBJ. Why will you need it?
};


#endif
