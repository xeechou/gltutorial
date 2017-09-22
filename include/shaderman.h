#ifndef SHADERMAN_H
#define SHADERMAN_H
#include <vector>
#include <map>
#include <string>
#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <utility>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <boost/filesystem.hpp>

#include "types.hpp"



//now it becomes the second place where we use the
class ShaderMan
{
protected:
	typedef boost::filesystem::path path_t;
	
	enum STYPE {VERTEX, FRAGMENT, GEOMETRY, COMPUTE};
	std::vector<GLuint> shaders;
	GLuint pid; //program id

	//I will call the static_callback first 
	int (* static_cb) (GLuint program);
	int (* perframe_cb) (GLuint program);
	//To shaders its only uniforms
public:
	//so, we have a brunch of aiSupported texture, for texture like CUBEMAP...
	//I need to figure out later how to support it.
	std::vector<TEX_TYPE> tex_uniforms;
	//default, vertex shader and fragment shader	
	int loadShaders(const char *, const char *);
	ShaderMan(void) {pid = 0;};
	//old interface
	ShaderMan(const char *vshader, const char *fshader);
	//add shader from string
	int addShader(const char *, STYPE type);
	int addShader(const path_t&, STYPE type);
	//int loadShader(boost::filesystem::path& p, STYPE shader_type);
	~ShaderMan();
	GLuint getPid(void) const {return pid;}
	void useProgram(void) {glUseProgram(pid);}
	//we need two callback
	virtual void setupTexUniform(void)  {};
};

//modulelize the shader



class TextureMan
{
	std::map<std::string, GLuint>textures;//is it a good idea to store textures by name? //
	size_t curr_texture;
	size_t actived_texture;
public:
	bool loadTexture(const char *fname, const char *ind_name);
	inline bool loadTexture(const char *fname) {
		return loadTexture(fname, fname);
	}
	bool activeTexture(const char *name);
	TextureMan(void) {curr_texture = 0; actived_texture = 0;}
	~TextureMan() {
		for (std::map<std::string, GLuint>::const_iterator it = textures.begin();
		     it != textures.end(); it++)
			glDeleteTextures(1, &(it->second));
	};
};





#endif
