#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utils.h>
#include <shaderman.h>
#include <string>

#include <controls.h>

//#include "controls.hpp"

const unsigned int width = 1024;
const unsigned int height = 1024;

glm::mat4 ROTATION = glm::mat4(1.0);
glm::mat4& get_rotation_mat(void) {
	return ROTATION;
}


//it do nothing for now
void tutorial_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glm::mat4 *model, *view, *projection;

	//we have a function for get/set current model, view, projection matrix.
	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
		
	} else if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		
	} else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
		
	} else if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		
	}
	
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	//the first position is the middle
	static double lxpos_prev = 0.0;
	static double lypos_prev = 0.0;
	static double rxpos_prev = 0.0;
	static double rypos_prev = 0.0;

	//I am not sure, I need to
	//okay, the xpos and ypos is in opencv-Coordinates
	xpos = (xpos - width / 2.0) / width;
	ypos = (height/ 2.0 - ypos) / height;
	int lstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	int rstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	
	if (lstate == GLFW_PRESS) {
		//moving_joint(window, lxpos_prev, lypos_prev, xpos, ypos);
		lxpos_prev = xpos; lypos_prev = ypos;
	} else if (rstate == GLFW_PRESS) {
		arcball_rotate(rxpos_prev, rypos_prev, xpos, ypos, get_rotation_mat());
		rxpos_prev = xpos; rypos_prev = ypos;
	}

}

//mesh loading functions
struct Vertex {
	glm::vec3 Position;
	glm::vec3 normal;
	glm::vec2 TexCoords;
};

/** UV mapping **/
struct Texture {
	GLuint id;
	std::string type;
};



class Mesh {
public:
	/* Mesh data */
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
private:
	GLuint VAO, VBO, EBO;

	void setupMesh();
};

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	this->setupMesh();
}



int main(void)
{
	//there are keypress callback and cursor callback function.
	GLFWwindow *window = tutorial_init(width, height, NULL, cursor_position_callback);
	
	//we have two object to draw
	ShaderMan container("vs.glsl", "fs.glsl");
	//create vertex array and vertex buffer

	GLuint prog_id = container.getPid();
	glUseProgram(prog_id);
	
	GLuint containerVAO, VBO;
	glGenVertexArrays(1, &containerVAO);
	glBindVertexArray(containerVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	//vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(1);
	//normal data
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat),
			      (GLvoid*)(sizeof(GL_FLOAT)*3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat),
			      (GLvoid*)(sizeof(GL_FLOAT)*6));
	glBindVertexArray(0);
	

	
	//fragment shader uniforms
	glUniform3f(glGetUniformLocation(prog_id, "objectColor"), 1.0f, 0.5f, 0.31f);
	glUniform3f(glGetUniformLocation(prog_id, "lightColor"), 1.0f, 1.0f, 1.0f); 
	glUniform3f(glGetUniformLocation(prog_id, "viewPos"), 4.0f, 3.0f, 3.0f); 
	//light meterial
	GLint matAmbientLoc  = glGetUniformLocation(prog_id, "light.ambient");
	GLint matDiffuseLoc  = glGetUniformLocation(prog_id, "light.diffuse");
	GLint matSpecularLoc = glGetUniformLocation(prog_id, "light.specular");
	GLint matShineLoc    = glGetUniformLocation(prog_id, "material.shininess");
	glUniform1f(matAmbientLoc,  0.3f);
	glUniform1f(matDiffuseLoc,  0.5f);
	glUniform1f(matSpecularLoc, 0.5f);
	glUniform1f(matShineLoc,    32.0f);

	//load texture: textures contains specular maps diffuse map
	TextureMan textures;
	if (!textures.loadTexture("../imgs/container2.png", "diffuse"))
		std::cout << "where is your img" << std::endl;
	textures.loadTexture("../imgs/container2-spec.png", "specular");

	glActiveTexture(GL_TEXTURE0);
	textures.activeTexture("diffuse");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glUniform1i(glGetUniformLocation(prog_id, "material.diffuse"), 0);
	
	//glActiveTexture(GL_TEXTURE1);
	textures.activeTexture("specular");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glUniform1i(glGetUniformLocation(prog_id, "material.specular"), 0);
 	glBindTexture(GL_TEXTURE_2D, 0);
	//materials, actually it is the object colors

	
	//second program
	ShaderMan light("lightvs.glsl", "lightfs.glsl");
	GLuint light_id = light.getPid();
	glUseProgram(light_id);
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//we don't need uniform on this one
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)0);
	glBindVertexArray(0);
	//load static uniforms light_id
	GLfloat theta = 0.0f;
	//no texture this time
	//there are some uniforms, in the vertex shader, we have to compute the uniform location

	do {
		glfwPollEvents();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//for container, the uniform data does not change, so we can apply now
		glm::mat4 Model = glm::mat4(1.0f);
		//Model = glm::scale(Model, glm::vec3(0.1f));
		glm::mat4 View  = glm::lookAt(glm::vec3(4,3,3), glm::vec3(0,0,0), glm::vec3(0,1,0)) * get_rotation_mat();
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		glm::mat4 mvp = Projection * View * Model;

		
		//light program
		
		glUseProgram(light_id);
		//compute the light pos at every step, the light is in 
		glm::vec3 light_pos(cos(theta), 1.0f, -sin(theta));
//		std::cout << "The light position is ["
//			  << light_pos[0] << ", "
//			  << light_pos[1] << ", "
//			  << light_pos[2] << "]" << std::endl;
		theta += 0.01f;
		glm::mat4 model = glm::translate(light_pos);
		model = glm::scale(model, glm::vec3(0.01f));
		glm::mat4 mvp2 = Projection * View * model;
		glUniformMatrix4fv(glGetUniformLocation(light_id, "mvp"), 1, GL_FALSE, &mvp2[0][0]);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//for the container
		glUseProgram(prog_id);
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		//std::cout << glGetUniformLocation(prog_id, "MVP") << std::endl;
		glUniformMatrix4fv(glGetUniformLocation(prog_id, "model"), 1, GL_FALSE, &Model[0][0]);

		glUniform3f(glGetUniformLocation(prog_id, "light.position"), light_pos[0], light_pos[1], light_pos[2]);
		glBindVertexArray(containerVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);


		
		glfwSwapBuffers(window);
		
	} while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
	       glfwWindowShouldClose(window) == 0 );
	
	//disable attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &containerVAO);
	glfwTerminate();
}
