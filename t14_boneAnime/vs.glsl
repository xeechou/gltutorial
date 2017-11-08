R"(
#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec2 bw0;
layout (location = 4) in vec2 bw1;
layout (location = 5) in vec2 bw2;
layout (location = 6) in vec3 bw3;

out vec2 TexCoords;
out vec3 fragPos;
out vec3 Normal;

//we couldn't really use a dynamic size of uniform array, since uniform size has
//limitations

const int maxNbone = 100;
uniform mat4 MVP;
uniform mat4 model;
uniform mat4 boneMat[maxNbone];

void main(void)
{
	//it is still very simple
	gl_Position = MVP * vec4(position, 1.0);
	Normal = normal;
	fragPos = vec3(model * vec4(position, 1.0f));
	TexCoords = texCoords;
}

)"

