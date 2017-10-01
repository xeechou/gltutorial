#include <boost/filesystem.hpp>
#include <utils.h>
#include <collections/drawobjs.hpp>
//you need to find a way to setup the cameras

namespace fs=boost::filesystem;

CubeMap::CubeMap(void)
{
	const std::string vs_source =
#include <collections/shaders/skyboxvs.glsl>
		;
	const std::string fs_source =
#include <collections/shaders/skyboxfs.glsl>
		;
	this->skybox_program.loadProgramFromString(vs_source, fs_source);
	this->prog = this->skybox_program.getPid();
}

CubeMap::~CubeMap()
{
	glDeleteTextures(1, &this->cube_tex);
}

bool
CubeMap::loadCubeMap(const fs::path& cubemaps)
{
	std::string faces[6];
	//load cubemaps information first, TODO remove this shitty code later
	{
		//okay, here is our first problem. on windows, the path becomes wchar_t, so we can't use
		std::string content;
		read_file(cubemaps.string(), content);
		//read the six lines
		std::stringstream ss(content);
		for (int i = 0; i < 6; i++) {
			std::getline(ss, faces[i]);
		}
	}

	GLuint texture_id;
	glActiveTexture(GL_TEXTURE0);	
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

	for (int i = 0; i < 6; i++) {
		fs::path dir = cubemaps.parent_path();
		cv::Mat img = cv::imread((dir / fs::path(faces[i])).string());
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
	this->cube_tex = texture_id;

	return true;
}

int CubeMap::init_setup(void)
{
	float skyboxVertices[] = {
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
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3*sizeof(float)), (GLvoid *)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	//setup p here
	this->p = glm::perspective(glm::radians(90.0f),
				   1.0f, 0.1f, 100.f);
	//setup texture

	//p is a constant, so we can use uniform here as well.
	return 0;
}

int
CubeMap::itr_setup()
{
	glm::mat4 v = glm::mat4(glm::mat3(this->get_camera_mat()));
	glm::mat4 pv = this->p * v;
	glUniformMatrix4fv(glGetUniformLocation(this->prog, "pv"), 1, GL_FALSE, &pv[0][0]);

	return 0;
}

int
CubeMap::itr_draw()
{
	glDepthFunc(GL_LEQUAL);
//	glDisable(GL_DEPTH_TEST);
	
	glBindVertexArray(this->vao);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
	GLuint cubetex_id = glGetUniformLocation(this->prog, "ctex");
	glUniform1i(cubetex_id, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
	return 0;
}
