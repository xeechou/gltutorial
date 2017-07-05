#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include <boost/filesystem.hpp>

#include <GL/glew.h>
#ifdef __linux__
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

static float skyboxVertices[] =
{
	// positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

namespace fs=boost::filesystem;

//this wil go to model class
unsigned int
loadCubeMap(const fs::path& cubemaps)
{
	std::string faces[6];
	//load cubemaps information first, TODO remove this shitty code later
	{
		std::string content;
		read_file(cubemaps.c_str(), &content);
		//read the six lines
		std::stringstream ss(content);
		for (int i = 0; i < 6; i++) {
			std::getline(ss, faces[i]);
		}
	}

	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	for (int i = 0; i < 6; i++) {
		fs::path dir = cubemaps.parent_path();
		cv::Mat img = cv::imread((dir / fs::path(faces[i])).c_str());
		if (!img.data) {
			std::cerr << "where is your cubemap image" << std::endl;
			return -1;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			     0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return texture_id;
}

void
skybox_geometry(GLuint& vao, GLuint& vbo)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3*sizeof(float)), (GLvoid *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int main(int argc, char **argv)
{
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = tutorial_init(width, height);
	glfwSetCursorPosCallback(window, unity_like_arcball_cursor);
	glfwSetScrollCallback(window, unity_like_arcball_scroll);
	
	ShaderMan container("vs.glsl", "fs.glsl");
	glUseProgram(container.getPid());
	GLuint prog_id = container.getPid();
	Model model(argv[1]);


	GLuint cubemap = loadCubeMap(argv[2]);
	GLuint cvao, cvbo;
	skybox_geometry(cvao, cvbo);
	ShaderMan skybox("skyboxvs.glsl", "skyboxfs.glsl");	
	
	GLfloat theta = 0.0f;
	do {
		glfwPollEvents();

		theta += 0.01f;

		glm::mat4 m = glm::mat4(0.01f);
		glm::mat4 v = unity_like_get_camera_mat();
		glm::mat4 p = glm::perspective(glm::radians(90.0f),
					       (float)width / (float)height,
					       0.1f, 100.f);
		glm::mat4 mvp = p * v * m;
		
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glEnable(GL_DEPTH_TEST);
//		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(prog_id);

		GLint matAmbientLoc  = glGetUniformLocation(prog_id, "light.ambient");
		GLint matDiffuseLoc  = glGetUniformLocation(prog_id, "light.diffuse");
		GLint matSpecularLoc = glGetUniformLocation(prog_id, "light.specular");
	
		glUniform1f(matAmbientLoc,  0.3f);
		glUniform1f(matDiffuseLoc,  0.5f);
		glUniform1f(matSpecularLoc, 0.5f);
		

		glm::vec3 light_pos(cos(theta), 5.0f, -sin(theta));

		glUniform3f(glGetUniformLocation(prog_id, "viewPos"), 4.0f, 3.0f, 3.0f);
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		//std::cout << glGetUniformLocation(prog_id, "MVP") << std::endl;
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "model"), 1, GL_FALSE, &m[0][0]);
		//light's other attributes are setted in other places
		glUniform3f(glGetUniformLocation(prog_id, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
		
//		fbobj.unreffbo();
//		glDisable(GL_DEPTH_TEST);
		model.draw(prog_id);



		glfwSwapBuffers(window);
		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	glfwTerminate();
}


