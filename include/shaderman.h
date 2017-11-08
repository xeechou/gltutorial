#ifndef SHADERMAN_H
#define SHADERMAN_H
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <tuple>

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif



#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <boost/filesystem.hpp>

#include "types.hpp"

//okay, we are gonna declare some global uniforms

//now it becomes the second place where we use the
//it will be nice if we can provide a string template and then use the string directly in the initializer
class ShaderMan
{
protected:
	typedef boost::filesystem::path path_t;
	
	enum STYPE {VERTEX, FRAGMENT, GEOMETRY, COMPUTE};
	std::vector<GLuint> shaders;
	GLuint pid; //program id

	//this is specificly for texture
	std::vector<std::pair<TEX_TYPE, std::string> > texture_uniforms;
	//including texture
	std::map<std::string, GLuint> uniforms;
	
public:
	ShaderMan(void) {pid = 0;};	
	ShaderMan(const char *vshader, const char *fshader);
	~ShaderMan(void);
	
	//so, we have a brunch of aiSupported texture, for texture like CUBEMAP...
	//I need to figure out later how to support it.
	//TODO: remove this
	std::vector<TEX_TYPE> tex_uniforms;
	//default, vertex shader and fragment shader	
	int loadShaders(const char *, const char *);
	//old interface

	void loadProgramFromString(const std::string&, const std::string&);
	//add shader from string
	int addShader(const char *, STYPE type);
	int addShader(const path_t&, STYPE type);
	GLuint getPid(void) const {return pid;}
	void useProgram(void) const {glUseProgram(pid);}
	//we need two callback
//	virtual void setupTexUniform(void)  {};
	//since we only support one texture per tex-type, we just need to query by the type
	bool addTextureUniform(const std::string name, const TEX_TYPE);
	//return texture id
	GLint getTexUniform(const TEX_TYPE type) const;
	//return uniform id
	GLint getUniform(const std::string name) const;
	
	static std::string getShaderName(GLenum shader);
	static GLuint createShaderFromString(const std::string& string, GLenum);
	static GLuint loadShaderProgram(GLuint *shaders, int len);
};

//modulelize the shader
GLint load2DTexture2GPU(const std::string fname);


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
