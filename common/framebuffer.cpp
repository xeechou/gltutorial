#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <fbobj.hpp>

static float QUADVERTICES[] = {
	// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f,  -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f,  -1.0f,  1.0f, 0.0f,
	1.0f,   1.0f,  1.0f, 1.0f
};


FBobject::FBobject(int width, int height)
{
	//when resolution changed. Detete this shit and use a new frambuffer
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);

	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUADVERTICES), QUADVERTICES, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4*sizeof(float)), (GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4*sizeof(float)),
			      (GLvoid *)(sizeof(float) * 2));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//create the texture
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &this->texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_INT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//to enable depth test, you have to allocate here as well. It is an option
	glGenRenderbuffers(1, &this->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->rbo);
	
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//don't allow user to know fbo where fbo is
void FBobject::reffbo()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
void FBobject::unreffbo()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void FBobject::drawfbo(GLuint prog)
{
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(prog);
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->texture);
	glUniform1i(glGetUniformLocation(prog, "ST"), 0);
	//draw on screen
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
		
}

FBobject::~FBobject(void)
{
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);
	glDeleteTextures(1, &texture);
}
