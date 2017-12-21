#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

#include <mutex>
#include <memory>
#include <thread>
#include <chrono>

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
#include <context.hpp>

#include <data.hpp>
#include <collections/shaders.hpp>
#include <collections/geometry.hpp>

const unsigned int width = 1024;
const unsigned int height = 1024;



class planeOBJ : public DrawObj {
private:
	phongNoShadow shader_program;
	std::shared_ptr<Model> model;
	int time;
	GLuint diffuse;
	GLuint specular;
public:
	planeOBJ(int param);
	int init_setup(void) override;
	int itr_setup(void) override;
	int itr_draw(void) override;
};

planeOBJ::planeOBJ(int param)
{
	//yep, we should setup the shader_program
	this->time = 0;
	this->model = std::make_shared<isoSphere>(1.0);
	this->model->addProperty("instancing",
				 std::make_shared<Instancing>(10, Instancing::OPTION::square_instances, glm::vec3(0.1f)));
	cv::Mat diffuse_texture = cv::Mat(500, 500, CV_8UC3, cv::Scalar(0,0,255));
	this->diffuse = load2DTexture2GPU(diffuse_texture);
	this->specular = load2DTexture2GPU(diffuse_texture);
}

int
planeOBJ::init_setup(void)
{
	return 0;
	this->model->push2GPU();
}

int
planeOBJ::itr_setup(void)
{
	//we can increase the time of drawing
	return 0;
	this->time += 0.01;
}

int
planeOBJ::itr_draw(void)
{
	this->shader_program.useProgram();
	glActiveTexture(GL_TEXTURE0 + this->shader_program.getTexUniform(TEX_Diffuse));
	glBindTexture(GL_TEXTURE_2D, this->diffuse);
	glActiveTexture(GL_TEXTURE0 + this->shader_program.getTexUniform(TEX_Specular));
	glBindTexture(GL_TEXTURE_2D, this->specular);
	glm::mat4 mpv = this->ctxt->getCameraMat();
	glm::vec3 pos = unity_like_get_view_pos();
	glUniformMatrix4fv(this->shader_program.getUniform(this->shader_program.uniform_MVP),
			   1, GL_FALSE, &mpv[0][0]);
	glUniform3f(this->shader_program.getUniform(this->shader_program.uniform_lightPos),
		    0.0f, 10.0f, 0.0f);
	this->model->drawProperty();
	return 0;
}

int main(int argc, char **argv)
{
	context cont(width, height, "window");
	cont.attachArcBallCamera(glm::radians(45.0f), glm::vec3(4.0,4.0,3.0f));
	GLFWwindow *window = cont.getGLFWwindow();
	planeOBJ obj(0);
	cont.append_drawObj(&obj);

//	ShaderMan cubeShader("vs.glsl", "fs.glsl");
//	ShaderMan shadowShader("lightvs.glsl", "lightfs.glsl");
	cont.init();
	cont.run();
}
