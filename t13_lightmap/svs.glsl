#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord;
layout (location = 3) in mat4 instancesMat;

uniform mat4 MVP;

void main(void)
{
	gl_Position = MVP * instancesMat * vec4(position, 1.0);
}
