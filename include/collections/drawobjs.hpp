#ifndef DRAW_COLLECTION_HPP
#define DRAW_COLLECTION_HPP

#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/filesystem.hpp>

#include "../context.hpp"
#include "../shaderman.h"


/**
 * @file drawCollection.hpp
 *
 * A collection of common drawing objects, such as skybox, shadowmap, to use this.
 */
class CubeMap : public DrawObj {
protected:
	GLuint cube_tex, vao, vbo;
	glm::mat4 p;
	ShaderMan skybox_program;
public:
	CubeMap(void);
	~CubeMap();
	bool loadCubeMap(const boost::filesystem::path& cubemaps);
	GLuint getCubeTex(void) const;
	int init_setup(void) override;
	//this get called first
	int itr_setup(void) override;
	//then this get called
	int itr_draw(void) override;
	glm::mat4 (*get_camera_mat)(void);
};

inline GLuint
CubeMap::getCubeTex() const
{
	return cube_tex;
}
	


#endif
