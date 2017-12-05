#ifndef TX_DATA_H
#define TX_DATA_H

//Quad for framebuffer objects
extern float QUADVERTICES[24];

extern float CUBEVERTS[108];
extern float CUBENORMS[108];
extern float CUBETEXS[72];

void bindQUAD(unsigned int vao, unsigned int vbo);
void drawQUAD(unsigned int vao, unsigned int vbo);
void bindCUBE(unsigned int vao, unsigned int vbo);
void drawCUBE(unsigned int vao, unsigned int vbo);

#include <Eigen/Core>
#include <Eigen/Dense>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static inline const aiMatrix4x4
glmMat2aiMat(const glm::mat4& mat)
{
     return aiMatrix4x4(mat[0][0],mat[0][1],mat[0][2],mat[0][3],
			mat[1][0],mat[1][1],mat[1][2],mat[1][3],
			mat[2][0],mat[2][1],mat[2][2],mat[2][3],
			mat[3][0],mat[3][1],mat[3][2],mat[3][3]);
}



static inline const glm::mat4
aiMat2glmMat(const aiMatrix4x4& in_mat)
{
      glm::mat4 tmp;
      tmp[0][0] = in_mat.a1;
      tmp[1][0] = in_mat.b1;
      tmp[2][0] = in_mat.c1;
      tmp[3][0] = in_mat.d1;

      tmp[0][1] = in_mat.a2;
      tmp[1][1] = in_mat.b2;
      tmp[2][1] = in_mat.c2;
      tmp[3][1] = in_mat.d2;

     tmp[0][2] = in_mat.a3;
     tmp[1][2] = in_mat.b3;
     tmp[2][2] = in_mat.c3;
     tmp[3][2] = in_mat.d3;

     tmp[0][3] = in_mat.a4;
     tmp[1][3] = in_mat.b4;
     tmp[2][3] = in_mat.c4;
     tmp[3][3] = in_mat.d4;
     return tmp;
}

template<typename T>
Eigen::Matrix<T, 4, 4>
glmMat2EigenMat(const glm::tmat4x4<T>& mat)
{
	Eigen::Matrix<T,4,4> ret;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			ret(i,j) = mat[i][j];
	return ret;
}

template<typename T>
glm::tmat4x4<T>
Eigen2glmMat(const Eigen::Matrix<T,4,4>& mat)
{
	glm::tmat4x4<T> ret;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			ret[i][j] = mat(i,j);
	return ret;
}
//extern float

#endif
