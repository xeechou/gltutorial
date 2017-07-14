#ifndef TYPES_HPP
#define TYPES_HPP

#include <utility>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

//well, we end up make those difficult type just to fix small bugs. Well, how
//can I say. This is definitly not complete. Very first example, we can't get a
//cubemap texture type here, but it is supported very well by OpenGL.
enum TEX_TYPE {
	TEX_Diffuse,
	TEX_Specular,
	TEX_Normal,
	TEX_Ambient,
	TEX_NTexType=4
};

typedef struct {
	aiTextureType aiTextype;
	TEX_TYPE ourTextype;
} sprt_tex2d_t;


//here is how you draw the texture if it is supported by assimp
//for
//for (int i = 0; i < shader->getSupported_texture_uniforms; i++)
//  glActiveTexture(GL_TEXTURE0 + shader->support_texture_start_indx + i)
//  glBindTexture(GL_TEXTURE_2D, matid_someting)

extern sprt_tex2d_t texture_types_supported[TEX_NTexType];


#endif /* EOF */
