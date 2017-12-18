#include <vector>
#include <GL/glew.h>
#ifdef __linux__
#include <GLFW/glfw3.h>
#elif __MINGW32__
#include <GLFW/glfw3.h>
#elif __WIN32
#include <GL/glfw3.h>
#endif
#include <Eigen/Core>
#include <glm/gtc/constants.hpp>

#include <data.hpp>
#include <operations.hpp>


float QUADVERTICES[24] = {
	// vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	1.0f,  -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	1.0f,  -1.0f,  1.0f, 0.0f,
	1.0f,   1.0f,  1.0f, 1.0f
};

float CUBEVERTS[108] = {
    // Back face
    -0.5f, -0.5f, -0.5f, // Bottom-left
     0.5f,  0.5f, -0.5f, // top-right
     0.5f, -0.5f, -0.5f, // bottom-right
     0.5f,  0.5f, -0.5f, // top-right
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f,  0.5f, -0.5f, // top-left
    // Front face
    -0.5f, -0.5f,  0.5f, // bottom-left
     0.5f, -0.5f,  0.5f, // bottom-right
     0.5f,  0.5f,  0.5f, // top-right
     0.5f,  0.5f,  0.5f, // top-right
    -0.5f,  0.5f,  0.5f, // top-left
    -0.5f, -0.5f,  0.5f, // bottom-left
    // Left face
    -0.5f,  0.5f,  0.5f, // top-right
    -0.5f,  0.5f, -0.5f, // top-left
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f, -0.5f, -0.5f, // bottom-left
    -0.5f, -0.5f,  0.5f, // bottom-right
    -0.5f,  0.5f,  0.5f, // top-right
    // Right face
     0.5f,  0.5f,  0.5f, // top-left
     0.5f, -0.5f, -0.5f, // bottom-right
     0.5f,  0.5f, -0.5f, // top-right
     0.5f, -0.5f, -0.5f, // bottom-right
     0.5f,  0.5f,  0.5f, // top-left
     0.5f, -0.5f,  0.5f, // bottom-left
    // Bottom face
    -0.5f, -0.5f, -0.5f, // top-right
     0.5f, -0.5f, -0.5f, // top-left
     0.5f, -0.5f,  0.5f, // bottom-left
     0.5f, -0.5f,  0.5f, // bottom-left
    -0.5f, -0.5f,  0.5f, // bottom-right
    -0.5f, -0.5f, -0.5f, // top-right
    // Top face
    -0.5f,  0.5f, -0.5f, // top-left
     0.5f,  0.5f,  0.5f, // bottom-right
     0.5f,  0.5f, -0.5f, // top-right
     0.5f,  0.5f,  0.5f, // bottom-right
    -0.5f,  0.5f, -0.5f, // top-left
    -0.5f,  0.5f,  0.5f, // bottom-left
};


float CUBENORMS[108] = {
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,

	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,

	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,

	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,

	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
};

float CUBETEXS[72] = {
    // Back face
    0.0f, 0.0f, // Bottom-left
    1.0f, 1.0f, // top-right
    1.0f, 0.0f, // bottom-right
    1.0f, 1.0f, // top-right
    0.0f, 0.0f, // bottom-left
    0.0f, 1.0f, // top-left
    // Front face
    0.0f, 0.0f, // bottom-left
    1.0f, 0.0f, // bottom-right
    1.0f, 1.0f, // top-right
    1.0f, 1.0f, // top-right
    0.0f, 1.0f, // top-left
    0.0f, 0.0f, // bottom-left
    // Left face
    1.0f, 0.0f, // top-right
    1.0f, 1.0f, // top-left
    0.0f, 1.0f, // bottom-left
    0.0f, 1.0f, // bottom-left
    0.0f, 0.0f, // bottom-right
    1.0f, 0.0f, // top-right
    // Right face
    1.0f, 0.0f, // top-left
    0.0f, 1.0f, // bottom-right
    1.0f, 1.0f, // top-right
    0.0f, 1.0f, // bottom-right
    1.0f, 0.0f, // top-left
    0.0f, 0.0f, // bottom-left
    // Bottom face
    0.0f, 1.0f, // top-right
    1.0f, 1.0f, // top-left
    1.0f, 0.0f, // bottom-left
    1.0f, 0.0f, // bottom-left
    0.0f, 0.0f, // bottom-right
    0.0f, 1.0f, // top-right
    // Top face
    0.0f, 1.0f, // top-left
    1.0f, 0.0f, // bottom-right
    1.0f, 1.0f, // top-right
    1.0f, 0.0f, // bottom-right
    0.0f, 1.0f, // top-left
    0.0f, 0.0f  // bottom-left
};

void
bindQUAD(unsigned int vao, unsigned int vbo)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUADVERTICES), QUADVERTICES, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4*sizeof(float)), (GLvoid *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4*sizeof(float)),
			      (GLvoid *)(sizeof(float) * 2));
	glBindVertexArray(0);
}

void
bindCUBE(unsigned int vao, unsigned int vbo)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	unsigned int stride = (3 + 3 + 2) * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, 36 * stride,
		     NULL, GL_STATIC_DRAW);

	int offset = 0;
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CUBEVERTS), CUBEVERTS);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (3*sizeof(float)), (GLvoid *)offset);
	offset += sizeof(CUBEVERTS);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(CUBENORMS), CUBENORMS);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (3*sizeof(float)),
			      (GLvoid *)offset);
	offset += sizeof(CUBENORMS);

	glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(CUBETEXS), CUBETEXS);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (2*sizeof(float)), ((GLvoid *)offset));
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawCUBE(unsigned int vao, unsigned int vbo)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//counts to number of vertex shader to run
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void drawQUAD(unsigned int vao, unsigned int vbo)
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//counts to number of vertex shader to run
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void bindSphere(unsigned int vao, unsigned int vbo, unsigned int ebo, float radius)
{
	//okay, we need to create the iso
	//create UV sphere, we will create a icosphere later
	float smallest_degree = 0.1 / radius;
	size_t nlats = (int)(glm::pi<float>() / smallest_degree);
	size_t nlongs = (int)(glm::pi<float>() * 2 / smallest_degree) - 1;
	size_t latitudes[nlats];
	int n = 0;
	std::generate(latitudes, latitudes+nlats, [&]() {
			return -glm::quarter_pi<float>() * 2.0 + smallest_degree * n++;
		});
	latitudes[nlats-1] = glm::quarter_pi<float>()*2;
	size_t longtitudes[nlongs];
	n = 0;
	std::generate(longtitudes, longtitudes+nlongs, [&]() {return smallest_degree * n++;});
	std::vector<glm::vec3> vertices((nlats-2) * nlongs + 2);
	vertices[0] = glm::vec3(0.0f, -radius, 0.0f);
	for (int i = 0; i < nlongs; i++) {
		for( int j = 1; j < nlats-1; j++) {
			int idx = i * (nlats-2) + j;
			vertices[idx] = polar2euclidean(radius, longtitudes[i], latitudes[j]);
		}
	}
	vertices[(nlats-2)*nlongs+1] = glm::vec3(0.0f, radius, 0.0f);
	//now generate the element buffer
	std::vector<glm::u32vec3> element_buffer;
	for (int i = 0; i < nlongs; i++) {
		int j = 1;
		int idx = i * (nlats-2)+j;
		int nextidx = ((i+1) % nlongs) * (nlats-2)+j;
		element_buffer.push_back(glm::u32vec3(idx, 0, nextidx));

		for (; j < nlats-2; j++) {
			idx = i * (nlats-2) + j;
			nextidx = ((i+1) % nlongs) *(nlats-2)+j;
			element_buffer.push_back(glm::u32vec3(idx, nextidx, nextidx+1));
			element_buffer.push_back(glm::u32vec3(nextidx+1, idx+1, idx));
		}
		idx = i * (nlats-2)+j;
		nextidx = ((i+1) % nlongs) * (nlats-2)+j;
		element_buffer.push_back(glm::u32vec3(idx, vertices[vertices.size()-1], nextidx));
	} //I need to test it

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(QUADVERTICES), QUADVERTICES, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(glm::vec3)), (GLvoid *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer.size() * sizeof(glm::u32vec3), &element_buffer[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
