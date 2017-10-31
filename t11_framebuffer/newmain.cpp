#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>


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


//my headers
#include <utils.h>
#include <shaderman.h>
#include <controls.h>
#include <model.hpp>
#include <fbobj.hpp>


const unsigned int width = 1024;
const unsigned int height = 1024;

class NanoShader : public ShaderMan {
public:
	NanoShader(void) {}
	void setupTexUniform(void) {
		glUseProgram(this->getPid());
		this->addTextureUniform("diffuse", TEX_Diffuse);
		this->addTextureUniform("specular", TEX_Specular);

//		GLuint diffuse_id  = glGetUniformLocation(this->getPid(), "diffuse");
//		GLuint specular_id = glGetUniformLocation(this->getPid(), "specular");
		//I should not need to active texture to 
//		glUniform1i(diffuse_id, 0);
//		glUniform1i(specular_id,1);
	}
};

int main(int argc, char **argv)
{
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = tutorial_init(width, height);
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);

	std::string quadvs =
#include "quadvs.glsl"
		;
	std::string quadfs =
#include "quadfs.glsl"
		;
	ShaderMan postprocess;
	postprocess.loadProgramFromString(quadvs, quadfs);
	glUseProgram(postprocess.getPid());
	FBobject fbobj(width, height);

	std::string vs_source =
#include "vs.glsl"
		;
	std::string fs_source =
#include "fs.glsl"
		;
	NanoShader container;
	container.loadProgramFromString(vs_source, fs_source);
	container.setupTexUniform();
	glUseProgram(container.getPid());
	GLuint prog_id = container.getPid();
	Model model;
	model.addProperty("mesh", std::make_shared<Mesh1>());
	model.addProperty("material", std::make_shared<Material1>());
	model.load(argv[1]);
//	model.push2GPU();
	
	model.bindShader(&container);

	GLfloat theta = 0.0f;
	do {
		glfwPollEvents();

		fbobj.reffbo();
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(prog_id);

		GLint matAmbientLoc  = glGetUniformLocation(prog_id, "light.ambient");
		GLint matDiffuseLoc  = glGetUniformLocation(prog_id, "light.diffuse");
		GLint matSpecularLoc = glGetUniformLocation(prog_id, "light.specular");
	
		glUniform1f(matAmbientLoc,  0.3f);
		glUniform1f(matDiffuseLoc,  0.5f);
		glUniform1f(matSpecularLoc, 0.5f);
		
		theta += 0.01f;

		glm::mat4 m = glm::mat4(0.01f);
		glm::mat4 v = unity_like_get_camera_mat();
		glm::mat4 p = glm::perspective(glm::radians(90.0f),
					       (float)width / (float)height,
					       0.1f, 100.f);
		glm::mat4 mvp = p * v * m;

		glm::vec3 light_pos(cos(theta), 5.0f, -sin(theta));

		glUniform3f(glGetUniformLocation(prog_id, "viewPos"), 4.0f, 3.0f, 3.0f);
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		//std::cout << glGetUniformLocation(prog_id, "MVP") << std::endl;
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "model"), 1, GL_FALSE, &m[0][0]);
		//light's other attributes are setted in other places
		glUniform3f(glGetUniformLocation(prog_id, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
		
//		fbobj.unreffbo();
//		glDisable(GL_DEPTH_TEST);
		model.drawProperty();

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" <<  std::endl;


		fbobj.unreffbo();
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		fbobj.drawfbo(postprocess.getPid());
		glfwSwapBuffers(window);
		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	glfwTerminate();
}
