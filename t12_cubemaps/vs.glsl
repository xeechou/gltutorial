#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in mat4 instanceMat;

out vec2 TexCoords;
out vec3 fragPos;
out vec3 Normal;

uniform mat4 MVP;
uniform mat4 model;

void main(void)
{
	//it is still very simple
	gl_Position = MVP * instanceMat * vec4(position, 1.0);
	Normal = normal;
	fragPos = vec3(model * vec4(position, 1.0f));
	TexCoords = texCoords;
}
